#include "eagle.h"
#include "esp_wifi.h"
#include "app/fs.h"

#define TAG __FILE_NAME__

extern const char version[] __attribute__((weak));
extern const char build_date[] __attribute__((weak));
extern const char wifi_format[] __attribute__((weak));
extern const char pass_format[] __attribute__((weak));
const char version[] ICACHE_RODATA_ATTR = "1.00";
const char build_date[] ICACHE_RODATA_ATTR = __DATE__ " " __TIME__;
const char wifi_format[] ICACHE_RODATA_ATTR = "ESP8266_%02X%02X%02X";
const char pass_format[] ICACHE_RODATA_ATTR = "8266ESP_%02X%02X%02X";
char thisname[16] = "";
char number[128] = "";

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
    }
}

void app_main()
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Component
    fs_init();

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
        config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        strcpy((char*)config.sta.ssid, fs_gets(number, 128, fd));
        strcpy((char*)config.sta.password, fs_gets(number, 128, fd));
        fs_close(fd);
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));
    }

    ESP_ERROR_CHECK(esp_wifi_start());
}
