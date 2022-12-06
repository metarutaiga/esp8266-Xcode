#include "esp8266.h"

static MQTT_Client* mqtt_client;
static bool mqtt_connected;
static char mqtt_ip[] PROGMEM = "192.168.1.1";

static char version[] PROGMEM = "1.00";
static char thisname[16];
static char number[128];

char* mqtt_prefix(char* pointer, const char* prefix, ...)
{
    va_list args;
    va_start(args, prefix);
    char* output = pointer;
    pointer += os_sprintf(pointer, PSTR("%s/%s"), thisname, prefix);
    while (const char* name = va_arg(args, char*))
    {
        pointer += os_sprintf(pointer, PSTR("/%s"), name);
    }
    va_end(args);
    return output;
}

void mqtt_information()
{
    struct ip_info ip_info = {};
    if (wifi_get_ip_info(STATION_IF, &ip_info))
    {
        os_sprintf(number, IPSTR, IP2STR(&ip_info.ip));
        MQTT_Publish(mqtt_client, mqtt_prefix(number + 64, PSTR("ESP"), PSTR("IP"), 0), number, 0, 0, 1);
    }

    if (struct rst_info* info = system_get_rst_info())
    {
        const char* reason;
        switch (info->reason)
        {
            case REASON_DEFAULT_RST:        reason = PSTR("Default");           break;
            case REASON_WDT_RST:            reason = PSTR("Hardware Watchdog"); break;
            case REASON_EXCEPTION_RST:      reason = PSTR("Exception");         break;
            case REASON_SOFT_WDT_RST:       reason = PSTR("Software Watchdog"); break;
            case REASON_SOFT_RESTART:       reason = PSTR("Software Restart");  break;
            case REASON_DEEP_SLEEP_AWAKE:   reason = PSTR("Deep-Sleep Awake");  break;
            case REASON_EXT_SYS_RST:        reason = PSTR("External System");   break;
            default:                        reason = PSTR("Unknown");           break;
        }
        MQTT_Publish(mqtt_client, mqtt_prefix(number, PSTR("ESP"), PSTR("ResetReason"), 0), reason, 0, 0, 0);

        if (info->reason >= REASON_WDT_RST && info->reason <= REASON_SOFT_WDT_RST)
        {
            char buff[256];
            os_sprintf(buff, PSTR("Fatal exception:%d flag:%d epc1:0x%08x epc2:0x%08x epc3:0x%08x excvaddr:0x%08x depc:0x%08x"), info->exccause, info->reason, info->epc1, info->epc2, info->epc3, info->excvaddr, info->depc);
            MQTT_Publish(mqtt_client, mqtt_prefix(number, PSTR("ESP"), PSTR("ResetInfo"), 0), buff, 0, 0, 0);
        }
        else
        {
            MQTT_Publish(mqtt_client, mqtt_prefix(number, PSTR("ESP"), PSTR("ResetInfo"), 0), reason, 0, 0, 0);
        }
    }

    MQTT_Publish(mqtt_client, mqtt_prefix(number, PSTR("ESP"), PSTR("Build"), 0), PSTR(__DATE__ " " __TIME__), 0, 0, 1);
    MQTT_Publish(mqtt_client, mqtt_prefix(number, PSTR("ESP"), PSTR("Version"), 0), version, 0, 0, 1);
}

void mqtt_loop()
{
    if (mqtt_connected == false)
        return;

    static uint32_t show = 0;
    if (show < system_get_time())
    {
        show = system_get_time() + 10 * 1000 * 1000;

        // Time
        static int now_timestamp = 0;
        uint32 timestamp = sntp_get_current_timestamp();
        if (now_timestamp != timestamp / 60)
        {
            now_timestamp = timestamp / 60;
            os_sprintf(number, PSTR("%d:%d"), timestamp / 3600 % 24, timestamp / 60 % 60);
            MQTT_Publish(mqtt_client, mqtt_prefix(number + 64, PSTR("ESP"), PSTR("Time"), 0), number, 0, 0, 0);
        }

        // Heap
        static int now_free_heap = 0;
        int free_heap = system_get_free_heap_size();
        if (now_free_heap != free_heap)
        {
            now_free_heap = free_heap;
            MQTT_Publish(mqtt_client, mqtt_prefix(number + 64, PSTR("ESP"), PSTR("FreeHeap"), 0), itoa(now_free_heap, number, 10), 0, 0, 0);
        }

        // RSSI
        static char now_rssi = 0;
        char rssi = wifi_station_get_rssi();
        if (now_rssi != rssi)
        {
            now_rssi = rssi;
            MQTT_Publish(mqtt_client, mqtt_prefix(number + 64, PSTR("ESP"), PSTR("RSSI"), 0), itoa(now_rssi, number, 10), 0, 0, 0);
        }
    }
}

void mqtt_setup()
{
    if (mqtt_client == nullptr)
    {
        mqtt_client = (MQTT_Client*)calloc(1, sizeof(MQTT_Client));
        MQTT_InitConnection(mqtt_client, mqtt_ip, 1883, 0);
        MQTT_InitClient(mqtt_client, thisname, 0, 0, 120, 1);
        MQTT_InitLWT(mqtt_client, mqtt_prefix(number, PSTR("connected"), 0), "false", 0, 1);
        MQTT_OnConnected(mqtt_client, [](uint32_t* args)
        {
            mqtt_connected = true;
            MQTT_Publish(mqtt_client, mqtt_prefix(number, PSTR("connected"), 0), PSTR("true"), 0, 0, 1);
            mqtt_information();
        });
        MQTT_OnDisconnected(mqtt_client, [](uint32_t* args)
        {
            mqtt_connected = false;
        });
        MQTT_OnData(mqtt_client, [](uint32_t* args, const char* topic, uint32_t topic_len, const char* data, uint32_t length)
        {
            
        });
    }
    MQTT_Connect(mqtt_client);
}

void setup(void)
{
    uint8 macaddr[6] = {};
    wifi_get_macaddr(STATION_IF, macaddr);
    os_sprintf(thisname, PSTR("ESP8266_%02X%02X%02X"), macaddr[3], macaddr[4], macaddr[5]);
    wifi_station_set_hostname(thisname);

    wifi_set_opmode_current(NULL_MODE);
    wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(0xFFFFFFF);
    wifi_set_opmode_current(STATIONAP_MODE);
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
            strcpy_P((char*)config.ssid, PSTR("wifi"));
            strcpy_P((char*)config.password, PSTR("1234"));
            config.all_channel_scan = true;

            wifi_set_opmode_current(STATIONAP_MODE);
            wifi_station_set_config(&config);
            wifi_station_connect();
            wifi_station_dhcpc_start();
            break;
        }
        case STATION_GOT_IP:
            wifi_set_opmode_current(STATION_MODE);
            mqtt_setup();
            sntp_setservername(0, "jp.pool.ntp.org");
            sntp_init();
            break;
        }
    }

    mqtt_loop();

    delay(100);
}
