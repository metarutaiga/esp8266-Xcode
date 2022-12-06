#include "c_types.h"
#include "osapi.h"
#include "user_interface.h"

void delay(unsigned int ms);

void setup(void)
{
    wifi_set_opmode_current(NULL_MODE);
    wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(0xFFFFFFF);
    wifi_set_opmode_current(STATIONAP_MODE);
}

void loop(void)
{
    static uint32_t show;
    if (show < system_get_time())
    {
        show = system_get_time() + 1 * 1000 * 1000;
        os_printf("[%10d] RAM : %d\n", system_get_time(), system_get_free_heap_size());
    }

    static uint8_t status = -1;
    if (status != wifi_station_get_connect_status())
    {
        status = wifi_station_get_connect_status();
        switch (status)
        {
        default:
        {
            struct station_config config = {};
            config.all_channel_scan = true;

            wifi_set_opmode_current(STATIONAP_MODE);
            wifi_station_set_config(&config);
            wifi_station_connect();
            wifi_station_dhcpc_start();
            break;
        }
        case STATION_GOT_IP:
            wifi_set_opmode_current(STATION_MODE);
            break;
        }
    }

    delay(100);
}
