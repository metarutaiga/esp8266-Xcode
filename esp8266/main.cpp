#include "esp8266.h"
#include <string>
#include "fs/fs.h"
#include "http.h"
#include "mqtt.h"

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
bool forceReset = false;

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

        fd = fs_open("ip", "r");
        if (fd >= 0)
        {
            struct ip_info info = {};
            info.ip.addr = ipaddr_addr(fs_gets(number, 128, fd));
            info.gw.addr = ipaddr_addr(fs_gets(number, 128, fd));
            info.netmask.addr = ipaddr_addr(fs_gets(number, 128, fd));
            ip_addr_t dns = { ipaddr_addr(fs_gets(number, 128, fd)) };

            wifi_set_ip_info(STATION_IF, &info);
            espconn_dns_setserver(0, &dns);

            fs_close(fd);
        }
    }

    // Soft AP
    struct softap_config config = {};
    wifi_softap_get_config_default(&config);
    config.ssid_len = os_sprintf((char*)config.ssid, wifi_format, macaddr[3], macaddr[4], macaddr[5]);
    os_sprintf((char*)config.password, pass_format, macaddr[3], macaddr[4], macaddr[5]);
    wifi_softap_set_config_current(&config);

    // Initialize
    wifi_set_opmode_current(NULL_MODE);
    wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(0xFFFFFFF);
}

void loop(void)
{
    static uint32_t show = 0;
    if (show < system_get_time())
    {
        show = system_get_time() + 1 * 1000 * 1000;
        char mode = 'B';
        switch (wifi_get_phy_mode())
        {
        case PHY_MODE_11B: mode = 'B'; break;
        case PHY_MODE_11G: mode = 'G'; break;
        case PHY_MODE_11N: mode = 'N'; break;
        }
        os_printf("[%10d] RAM : %d %c\n", system_get_time(), system_get_free_heap_size(), mode);
    }

    static uint8_t status = 0;
    if (status != wifi_station_get_connect_status() + 1)
    {
        status = wifi_station_get_connect_status() + 1;
        switch (status - 1)
        {
        default:
        {
            wifi_set_opmode_current(STATIONAP_MODE);
            wifi_station_set_hostname(thisname);
            wifi_station_connect();
            wifi_station_dhcpc_start();

            // HTTP
            http_regist("/", "text/html", web_system);
            break;
        }
        case STATION_GOT_IP:
            wifi_set_opmode_current(STATION_MODE);
            wifi_station_set_hostname(thisname);

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

            // HTTP
            http_regist("/", "text/html", web_system);
            break;
        }
    }

    // ForceReset
    if (forceReset)
    {
        forceReset = false;
        system_restart();
    }

    mqtt_loop();

    delay(100);
}
