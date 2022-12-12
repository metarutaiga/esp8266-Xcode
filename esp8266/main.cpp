#include "esp8266.h"
#include "mqtt.h"

static char mqtt_ip[] PROGMEM = "192.168.1.1";
static char wifi_ssid[] PROGMEM = "wifi";
static char wifi_pass[] PROGMEM = "1234";
static char wifi_format[] PROGMEM = "ESP8266_%02X%02X%02X";

extern const char version[] = "1.00";
char thisname[16] IRAM_ATTR = "";
char number[128] IRAM_ATTR = "";

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
}

void loop(void)
{
    static uint32_t show = 0;
    if (show < system_get_time())
    {
        show = system_get_time() + 1 * 1000 * 1000;
        os_printf("[%10d] RAM : %d\n", system_get_time(), system_get_free_heap_size());
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
            strcpy((char*)config.ssid, wifi_ssid);
            strcpy((char*)config.password, wifi_pass);
            config.all_channel_scan = true;

            wifi_set_opmode_current(STATIONAP_MODE);
            wifi_station_set_config(&config);
            wifi_station_connect();
            wifi_station_dhcpc_start();
            break;
        }
        case STATION_GOT_IP:
            wifi_set_opmode_current(STATION_MODE);
            mqtt_setup(mqtt_ip, 1883);
            sntp_setservername(0, "jp.pool.ntp.org");
            sntp_init();
            break;
        }
    }

    mqtt_loop();

    delay(100);
}
