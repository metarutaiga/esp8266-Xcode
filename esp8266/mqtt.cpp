#include "esp8266.h"
#include "mqtt.h"

extern "C"
{
#   define BOOL bool
#   define uint8_t const char
#   include "../mqtt/include/mqtt.h"
#   undef BOOL
#   undef uint8_t
#   define MQTT_Publish(client, topic, data_, data_length_, qos, retain) \
    { \
        const char* data = data_; \
        int data_length = data_length_ ? data_length_ : os_strlen(data); \
        MQTT_Publish(client, topic, data, data_length, qos, retain); \
    }
};

static MQTT_Client* mqtt_client;
static bool mqtt_connected;

char* mqtt_prefix(char* pointer, const char* prefix, ...)
{
    va_list args;
    va_start(args, prefix);
    char* output = pointer;
    pointer += os_sprintf(pointer, "%s", thisname);
    pointer += os_sprintf(pointer, "/%s", prefix);
    while (const char* name = va_arg(args, char*))
    {
        pointer += os_sprintf(pointer, "/%s", name);
    }
    va_end(args);
    return output;
}

void mqtt_publish(const char* topic, const void* data, int length)
{
    MQTT_Publish(mqtt_client, topic, data, length, 0, 0);
}

void mqtt_information()
{
    MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "SDK Version", 0), system_get_sdk_version(), 0, 0, 0);
    MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "CPU Frequency", 0), itoa(system_get_cpu_freq(), number + 64, 10), 0, 0, 0);

    MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "Build", 0), __DATE__ " " __TIME__, 0, 0, 0);
    MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "Version", 0), version, 0, 0, 1);
    struct ip_info ip_info = {};
    if (wifi_get_ip_info(STATION_IF, &ip_info))
    {
        os_sprintf(number + 64, IPSTR, IP2STR(&ip_info.ip));
        MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "IP", 0), number + 64, 0, 0, 1);
    }

    if (struct rst_info* info = system_get_rst_info())
    {
        const char* reason;
        switch (info->reason)
        {
            case REASON_DEFAULT_RST:        reason = "Default";             break;
            case REASON_WDT_RST:            reason = "Hardware Watchdog";   break;
            case REASON_EXCEPTION_RST:      reason = "Exception";           break;
            case REASON_SOFT_WDT_RST:       reason = "Software Watchdog";   break;
            case REASON_SOFT_RESTART:       reason = "Software Restart";    break;
            case REASON_DEEP_SLEEP_AWAKE:   reason = "Deep-Sleep Awake";    break;
            case REASON_EXT_SYS_RST:        reason = "External System";     break;
            default:                        reason = "Unknown";             break;
        }
        MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "ResetReason", 0), reason, 0, 0, 0);

        if (info->reason >= REASON_WDT_RST && info->reason <= REASON_SOFT_WDT_RST)
        {
            char buff[256];
            os_sprintf(buff, "Exception:%d flag:%d epc1:0x%08x epc2:0x%08x epc3:0x%08x excvaddr:0x%08x depc:0x%08x", info->exccause, info->reason, info->epc1, info->epc2, info->epc3, info->excvaddr, info->depc);
            MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "ResetInfo", 0), buff, 0, 0, 0);
        }
        else
        {
            MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "ResetInfo", 0), reason, 0, 0, 0);
        }
    }
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
        static uint32 now_timestamp = 0;
        uint32 timestamp = sntp_get_current_timestamp();
        if (now_timestamp != timestamp / 60)
        {
            now_timestamp = timestamp / 60;
            os_sprintf(number + 64, "%d:%d", timestamp / 3600 % 24, timestamp / 60 % 60);
            MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "Time", 0), number + 64, 0, 0, 0);
        }

        // Heap
        static uint16 now_free_heap = 0;
        uint16 free_heap = system_get_free_heap_size();
        if (now_free_heap != free_heap)
        {
            now_free_heap = free_heap;
            MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "FreeHeap", 0), itoa(free_heap, number + 64, 10), 0, 0, 0);
        }

        // RSSI
        static sint8 now_rssi = 0;
        sint8 rssi = wifi_station_get_rssi();
        if (now_rssi != rssi)
        {
            now_rssi = rssi;
            MQTT_Publish(mqtt_client, mqtt_prefix(number, "ESP", "RSSI", 0), itoa(rssi | 0xFFFFFF00, number + 64, 10), 0, 0, 0);
        }
    }
}

void mqtt_setup(const char* ip, int port)
{
    if (mqtt_client == nullptr)
    {
        mqtt_client = (MQTT_Client*)calloc(1, sizeof(MQTT_Client));
        MQTT_InitConnection(mqtt_client, ip, port, 0);
        MQTT_InitClient(mqtt_client, thisname, 0, 0, 120, 1);
        MQTT_InitLWT(mqtt_client, mqtt_prefix(number, "connected", 0), "false", 0, 1);
        MQTT_OnConnected(mqtt_client, [](uint32_t* args)
        {
            mqtt_connected = true;
            MQTT_Publish(mqtt_client, mqtt_prefix(number, "connected", 0), "true", 0, 0, 1);
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
