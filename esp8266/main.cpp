#include "esp8266.h"
#include <string>
#include "app/fs.h"
#include "app/homekit.h"
#include "app/httpd.h"
#include "app/https.h"
#include "app/mqtt.h"
#include "app/rtc.h"
#include "app/ota.h"
#include "app/uart.h"

extern "C" void ets_write_char(char c);

extern bool web_system(void* arg, const char* url, int line);
extern bool web_ssid(void* arg, const char* url, int line);
extern bool web_ip(void* arg, const char* url, int line);
extern bool web_ota(void* arg, const char* url, int line);
extern bool web_mqtt(void* arg, const char* url, int line);
extern bool web_ntp(void* arg, const char* url, int line);
extern bool web_reset(void* arg, const char* url, int line);
extern bool web_rtc(void* arg, const char* url, int line);

extern const char version[] __attribute__((weak));
extern const char build_date[] __attribute__((weak));
extern const char wifi_format[] __attribute__((weak));
extern const char pass_format[] __attribute__((weak));
extern const char version[] ICACHE_RODATA_ATTR = "1.00";
extern const char build_date[] ICACHE_RODATA_ATTR = __DATE__ " " __TIME__;
extern const char wifi_format[] ICACHE_RODATA_ATTR = "ESP8266_%02X%02X%02X";
extern const char pass_format[] ICACHE_RODATA_ATTR = "8266ESP_%02X%02X%02X";
char thisname[16] = "";
char number[128] = "";

extern "C" void app_wifi(System_Event_t* event) __attribute__((weak));
extern "C" void app_wifi(System_Event_t* event) {}
void wifi(System_Event_t* event)
{
    uint32_t type = event ? event->event : EVENT_STAMODE_DISCONNECTED;
    switch (type)
    {
    case EVENT_STAMODE_GOT_IP:
    {
        wifi_set_opmode(STATION_MODE);
        wifi_station_set_hostname(thisname);

        // HTTP
        httpd_regist("/setup", "text/html", web_system);
#ifdef DEMO
        // HTTPS
        https_connect("https://raw.githubusercontent.com/metarutaiga/esp8266-Xcode/master/LICENSE.txt", [](void* arg, char* data, int length)
        {
            for (int i = 0; i < length; ++i)
            {
                ets_write_char(data[i]);
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
        string zone = "8";
        fd = fs_open("ntp", "r");
        if (fd >= 0)
        {
            ntp = fs_gets(number, 128, fd);
            zone = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        sntp_setservername(0, ntp.data());
        sntp_set_timezone(strtol(zone.c_str(), nullptr, 10));
        sntp_init();

        // OTA
        fd = fs_open("ota", "r");
        if (fd >= 0)
        {
            if (os_strcmp(fs_gets(number, 128, fd), "YES") == 0)
                ota_init(8266);
            fs_close(fd);
        }

        // HomeKit
        homekit_init();
        break;
    }
    case EVENT_STAMODE_CONNECTED:
    case EVENT_STAMODE_DISCONNECTED:
    case EVENT_STAMODE_DHCP_TIMEOUT:
        wifi_set_opmode(STATIONAP_MODE);
        wifi_station_set_hostname(thisname);
        wifi_station_connect();
        wifi_station_dhcpc_start();

        // HTTP
        httpd_regist("/", "text/html", web_system);
        break;
    }
    app_wifi(event);
}

extern "C" void app_setup() __attribute__((weak));
extern "C" void app_setup() {}
void setup(void)
{
    // Component
    gpio_init();
    fs_init();
    httpd_init(80);
    httpd_regist("/", "text/html", web_system);
    httpd_regist("/ssid", nullptr, web_ssid);
    httpd_regist("/ip", nullptr, web_ip);
    httpd_regist("/ota", nullptr, web_ota);
    httpd_regist("/mqtt", nullptr, web_mqtt);
    httpd_regist("/ntp", nullptr, web_ntp);
    httpd_regist("/reset", nullptr, web_reset);
    httpd_regist("/rtc", "text/plain; charset=utf-8", web_rtc);
    rtc_begin();

    // MAC
    uint8 macaddr[6] = {};
    wifi_get_macaddr(STATION_IF, macaddr);

    // Soft AP
    struct softap_config config = {};
    os_sprintf((char*)config.ssid, wifi_format, macaddr[3], macaddr[4], macaddr[5]);
    os_sprintf((char*)config.password, pass_format, macaddr[3], macaddr[4], macaddr[5]);
    config.ssid_len = os_strlen((char*)config.ssid);
    config.channel = 1;
    config.authmode = AUTH_WPA_WPA2_PSK;
    config.ssid_hidden = 0;
    config.max_connection = 4;
    config.beacon_interval = 100;
    wifi_softap_set_config(&config);
    os_strcpy(thisname, (char*)config.ssid);

    // SSID
    int fd = fs_open("ssid", "r");
    if (fd >= 0)
    {
        struct station_config config = {};
        config.all_channel_scan = true;
        os_strcpy((char*)config.ssid, fs_gets(number, 128, fd));
        os_strcpy((char*)config.password, fs_gets(number, 128, fd));
        fs_close(fd);
        wifi_station_set_config(&config);

        // Static IP
        fd = fs_open("ip", "r");
        if (fd >= 0)
        {
            struct ip_info info = {};
            info.ip.addr = ipaddr_addr(fs_gets(number, 128, fd));
            info.gw.addr = ipaddr_addr(fs_gets(number, 128, fd));
            info.netmask.addr = ipaddr_addr(fs_gets(number, 128, fd));
            ip_addr_t dns = { ipaddr_addr(fs_gets(number, 128, fd)) };
            fs_close(fd);

            wifi_set_ip_info(STATION_IF, &info);
            espconn_dns_setserver(0, &dns);
        }
    }
#ifdef DEMO
    // UART
    uart_init(2, 0, 9600, 8, NULL, 1, 256);
#endif
    // Initialize
    wifi_set_event_handler_cb(wifi);
    wifi(nullptr);
#ifdef DEMO
    // Debug
    static os_timer_t timer IRAM_ATTR;
    os_timer_setfn(&timer, [](void* arg)
    {
        char mode = 'B';
        switch (wifi_get_phy_mode())
        {
        case PHY_MODE_11B: mode = 'B'; break;
        case PHY_MODE_11G: mode = 'G'; break;
        case PHY_MODE_11N: mode = 'N'; break;
        }
        os_printf("[%6d.%04d] RAM : %d %c\n",
                  uint32_t(system_get_time64() / 1000000),
                  uint32_t(system_get_time64() % 1000000),
                  system_get_free_heap_size(), mode);
    }, &timer);
    os_timer_arm(&timer, 1000, true);
#endif
    app_setup();
}

#ifdef LOOP
void loop(void)
{
    delay(100);
}
#endif
