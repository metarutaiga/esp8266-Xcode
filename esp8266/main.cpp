#include "eagle.h"
#include <driver/gpio.h>
#include <esp_http_server.h>
#include <lwip/apps/sntp.h>
#include <nvs_flash.h>
#include "app/fs.h"
#include "app/gpio.h"
#include "app/https.h"
#include "app/mqtt.h"
#include "app/ota.h"
#include "app/rtc.h"
#include "app/uart.h"

#define TAG __FILE_NAME__

extern const char version[] __attribute__((weak));
extern const char build_date[] __attribute__((weak));
extern const char web_css[] __attribute__((weak));
extern const char wifi_format[] __attribute__((weak));
extern const char pass_format[] __attribute__((weak));
const char version[] RODATA_STR_ATTR = "1.00";
const char build_date[] RODATA_STR_ATTR = __DATE__ " " __TIME__;
const char web_css[] RODATA_STR_ATTR = "";
const char wifi_format[] RODATA_STR_ATTR = "ESP8266_%02X%02X%02X";
const char pass_format[] RODATA_STR_ATTR = "8266ESP_%02X%02X%02X";
char thisname[24] = "";
char number[128] = "";
bool debug;

extern "C" const char* const __wrap_default_ssid = wifi_format;

httpd_handle_t httpd_server IRAM_BSS_ATTR;
extern esp_err_t web_system(httpd_req_t* req);
extern esp_err_t web_ssid(httpd_req_t* req);
extern esp_err_t web_ota(httpd_req_t* req);
extern esp_err_t web_mqtt(httpd_req_t* req);
extern esp_err_t web_ntp(httpd_req_t* req);
extern esp_err_t web_reset(httpd_req_t* req);
extern esp_err_t web_rtc(httpd_req_t* req);

extern "C" void app_wifi() __attribute__((weak));
extern "C" void app_wifi() {}
static void wifi_handler(void)
{
    // HTTP
    httpd_uri_t web_system_uri = { .uri = "/setup", .method = HTTP_GET, .handler = web_system };
    httpd_register_uri_handler(httpd_server, &web_system_uri);
#ifdef DEMO
    // HTTPS
    https_connect("https://raw.githubusercontent.com/metarutaiga/esp8266-Xcode/master/LICENSE.txt", "Connection: close", [](void* arg, char* data, int length)
    {
        for (int i = 0; i < length; ++i)
        {
            ets_putc(data[i]);
        }
    }, nullptr);
#endif
    // MQTT
    int fd = fs_open("mqtt", "r");
    if (fd >= 0)
    {
        string mqtt = fs_gets(number, 128, fd);
        string port = fs_gets(number, 128, fd);
        mqtt_setup(mqtt.c_str(), strtol(port.c_str(), nullptr, 10));
        fs_close(fd);
    }

    // NTP
    string ntp = "pool.ntp.org";
    string zone = "GMT-8";
    fd = fs_open("ntp", "r");
    if (fd >= 0)
    {
        ntp = fs_gets(number, 128, fd);
        zone = fs_gets(number, 128, fd);
        fs_close(fd);
    }
    free((void*)sntp_getservername(0));
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, strdup(ntp.c_str()));
    sntp_init();
    setenv("TZ", zone.c_str(), 1);
    tzset();

    // OTA
    fd = fs_open("ota", "r");
    if (fd >= 0)
    {
        if (strcmp(fs_gets(number, 128, fd), "YES") == 0)
        {
            ota_init(8266);
            debug = true;
        }
        fs_close(fd);
    }

    app_wifi();

    // Dump task
    size_t uxArraySize = uxTaskGetNumberOfTasks();
    TaskStatus_t* pxTaskStatusArray = (TaskStatus_t*)malloc(uxArraySize * sizeof(TaskStatus_t));
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, nullptr);
    for (int i = 0; i < uxArraySize; ++i)
    {
        ESP_LOGI(TAG, "%d: %p %8d %s", pxTaskStatusArray[i].xTaskNumber, pxTaskStatusArray[i].pxStackBase, pxTaskStatusArray[i].usStackHighWaterMark, pxTaskStatusArray[i].pcTaskName);
    }
    free(pxTaskStatusArray);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

        wifi_handler();
    }
}

extern "C" void app_setup() __attribute__((weak));
extern "C" void app_setup() {}
extern "C" void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Component
    ESP_LOGI(TAG, build_date);
    fs_init();
    gpio_init();
    rtc_begin();

    // WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    // MAC
    uint8_t macaddr[6] = {};
    esp_wifi_get_mac(ESP_IF_WIFI_STA, macaddr);

    // Soft AP
    wifi_config_t config = {};
    sprintf((char*)config.ap.ssid, wifi_format, macaddr[3], macaddr[4], macaddr[5]);
    sprintf((char*)config.ap.password, pass_format, macaddr[3], macaddr[4], macaddr[5]);
    config.ap.ssid_len = strlen((char*)config.ap.ssid);
    config.ap.channel = 1;
    config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    config.ap.ssid_hidden = 0;
    config.ap.max_connection = 4;
    config.ap.beacon_interval = 100;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &config));
    strcpy(thisname, (char*)config.ap.ssid);

    // SSID
    int fd = fs_open("ssid", "r");
    if (fd >= 0)
    {
        wifi_config_t config = {};
        strcpy((char*)config.sta.ssid, fs_gets(number, 128, fd));
        strcpy((char*)config.sta.password, fs_gets(number, 128, fd));
        fs_close(fd);
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));
    }

    ESP_ERROR_CHECK(esp_wifi_start());

    // Start the httpd server
    httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(TAG, "Starting server on port: '%d'", httpd_config.server_port);
    httpd_start(&httpd_server, &httpd_config);

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_uri_t web_system_uri = { .uri = "/", .method = HTTP_GET, .handler = web_system };
    httpd_uri_t web_ssid_uri = { .uri = "/ssid", .method = HTTP_GET, .handler = web_ssid };
    httpd_uri_t web_ota_uri = { .uri = "/ota", .method = HTTP_GET, .handler = web_ota };
    httpd_uri_t web_mqtt_uri = { .uri = "/mqtt", .method = HTTP_GET, .handler = web_mqtt };
    httpd_uri_t web_ntp_uri = { .uri = "/ntp", .method = HTTP_GET, .handler = web_ntp };
    httpd_uri_t web_reset_uri = { .uri = "/reset", .method = HTTP_GET, .handler = web_reset };
    httpd_uri_t web_rtc_uri = { .uri = "/rtc", .method = HTTP_GET, .handler = web_rtc, .user_ctx = (void*)"text/plain; charset=utf-8" };
    httpd_register_uri_handler(httpd_server, &web_system_uri);
    httpd_register_uri_handler(httpd_server, &web_ssid_uri);
    httpd_register_uri_handler(httpd_server, &web_ota_uri);
    httpd_register_uri_handler(httpd_server, &web_mqtt_uri);
    httpd_register_uri_handler(httpd_server, &web_ntp_uri);
    httpd_register_uri_handler(httpd_server, &web_reset_uri);
    httpd_register_uri_handler(httpd_server, &web_rtc_uri);
#ifdef DEMO
    // UART
    uart_init(2, 0, 9600, 8, NULL, 1, 256);
#endif
    app_setup();
}
