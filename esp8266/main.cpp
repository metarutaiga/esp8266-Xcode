#include "esp8266.h"
#include <string>
#include "app/fs.h"
#include "app/http.h"
#include "app/mqtt.h"
#include "app/ota.h"

extern bool web_system(void* arg, const char* url, int line);
extern bool web_ssid(void* arg, const char* url, int line);
extern bool web_ip(void* arg, const char* url, int line);
extern bool web_ota(void* arg, const char* url, int line);
extern bool web_mqtt(void* arg, const char* url, int line);
extern bool web_ntp(void* arg, const char* url, int line);
extern bool web_reset(void* arg, const char* url, int line);

static const char wifi_format[] = "ESP8266_%02X%02X%02X";
static const char pass_format[] = "8266ESP_%02X%02X%02X";
extern const char version[] = "1.00";
char thisname[16] = "";
char number[128] = "";

void wifi(System_Event_t* event)
{
    uint32_t type = event ? event->event : EVENT_STAMODE_DISCONNECTED;
    switch (type)
    {
    case EVENT_STAMODE_GOT_IP:
    {
        wifi_set_opmode_current(STATION_MODE);
        wifi_station_set_hostname(thisname);

        // HTTP
        http_regist("/", "text/html", web_system);

        // MQTT
        int fd = fs_open("mqtt", "r");
        if (fd >= 0)
        {
            std::string mqtt = fs_gets(number, 128, fd);
            std::string port = fs_gets(number, 128, fd);
            mqtt_setup(mqtt.c_str(), strtol(port.c_str(), nullptr, 10));
            fs_close(fd);
        }

        // NTP
        std::string ntp = "pool.ntp.org";
        std::string zone = "8";
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
            if (strcmp(fs_gets(number, 128, fd), "YES") == 0)
                ota_init(8266);
            fs_close(fd);
        }
        break;
    }
    case EVENT_STAMODE_CONNECTED:
    case EVENT_STAMODE_DISCONNECTED:
    case EVENT_STAMODE_DHCP_TIMEOUT:
        wifi_set_opmode_current(STATIONAP_MODE);
        wifi_station_set_hostname(thisname);
        wifi_station_connect();
        wifi_station_dhcpc_start();

        // HTTP
        http_regist("/", "text/html", web_system);
        break;
    }
}

void setup(void)
{
    // Component
    fs_init();
    http_init(80);
    http_regist("/", "text/html", web_system);
    http_regist("/ssid", nullptr, web_ssid);
    http_regist("/ip", nullptr, web_ip);
    http_regist("/ota", nullptr, web_ota);
    http_regist("/mqtt", nullptr, web_mqtt);
    http_regist("/ntp", nullptr, web_ntp);
    http_regist("/reset", nullptr, web_reset);

    // Hostname
    uint8 macaddr[6] = {};
    wifi_get_macaddr(STATION_IF, macaddr);
    os_sprintf(thisname, wifi_format, macaddr[3], macaddr[4], macaddr[5]);

    // Soft AP
    struct softap_config config = {};
    wifi_softap_get_config_default(&config);
    config.ssid_len = os_sprintf((char*)config.ssid, wifi_format, macaddr[3], macaddr[4], macaddr[5]);
    os_sprintf((char*)config.password, pass_format, macaddr[3], macaddr[4], macaddr[5]);
    wifi_softap_set_config_current(&config);

    // SSID
    int fd = fs_open("ssid", "r");
    if (fd >= 0)
    {
        struct station_config config = {};
        config.all_channel_scan = true;
        os_strcpy((char*)config.ssid, fs_gets(number, 128, fd));
        os_strcpy((char*)config.password, fs_gets(number, 128, fd));
        fs_close(fd);

        wifi_station_set_config_current(&config);

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

    // Initialize
    wifi_set_event_handler_cb(wifi);
    wifi(nullptr);

    // Debug
    static os_timer_t timer;
    os_timer_setfn(&timer, [](void* arg)
    {
        char mode = 'B';
        switch (wifi_get_phy_mode())
        {
        case PHY_MODE_11B: mode = 'B'; break;
        case PHY_MODE_11G: mode = 'G'; break;
        case PHY_MODE_11N: mode = 'N'; break;
        }
        os_printf("[%10d] RAM : %d %c\n", system_get_time(), system_get_free_heap_size(), mode);
    }, &timer);
    os_timer_arm(&timer, 1000, true);
}

void loop(void)
{
    delay(100);
}
