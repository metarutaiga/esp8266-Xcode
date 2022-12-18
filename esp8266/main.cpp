#include "esp8266.h"
#include <string>
#include "fs/fs.h"
#include "http.h"
#include "mqtt.h"

static char wifi_format[] PROGMEM = "ESP8266_%02X%02X%02X";

extern const char version[] = "1.00";
char thisname[16] = "";
char number[128] = "";

void setup(void)
{
    uint8 macaddr[6] = {};
    wifi_get_macaddr(STATION_IF, macaddr);
    os_sprintf(thisname, wifi_format, macaddr[3], macaddr[4], macaddr[5]);
    wifi_station_set_hostname(thisname);

    wifi_set_opmode_current(NULL_MODE);
    wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(0xFFFFFFF);

    fs_init();
    http_init(80);

    extern bool web_system(void* arg, const char* url, int line);
    extern bool web_ssid(void* arg, const char* url, int line);

    http_regist("/", "text/html", web_system);
    http_regist("/ssid", nullptr, web_ssid);
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
            struct station_config config = {};
            config.all_channel_scan = true;

            // SSID
            int fd = fs_open("ssid", "r");
            if (fd >= 0)
            {
                os_strcpy((char*)config.ssid, fs_gets(number, 128, fd));
                os_strcpy((char*)config.password, fs_gets(number, 128, fd));
                fs_close(fd);
            }
            fd = fs_open("ip", "r");
            if (fd >= 0)
            {
                std::string ip = fs_gets(number, 128, fd);
                std::string gateway = fs_gets(number, 128, fd);
                std::string subnet = fs_gets(number, 128, fd);
                std::string dns = fs_gets(number, 128, fd);
                fs_close(fd);
            }

            wifi_set_opmode_current(STATIONAP_MODE);
            wifi_station_set_config(&config);
            wifi_station_connect();
            wifi_station_dhcpc_start();
            break;
        }
        case STATION_GOT_IP:
            wifi_set_opmode_current(STATION_MODE);

            // MQTT
            int fd = fs_open("ssid", "r");
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
            break;
        }
    }

    mqtt_loop();

    delay(100);
}
