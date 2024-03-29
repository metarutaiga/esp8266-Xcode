#include "esp8266.h"
#include "mqtt.h"

extern "C"
{
#   define BOOL bool
#   define uint8_t const char
#   include <mqtt/include/mqtt.h>
#   undef BOOL
#   undef uint8_t
};

static MQTT_Client* mqtt_client IRAM_ATTR;
static void (*mqtt_receive_callback)(const char* topic, uint32_t topic_len, const char* data, uint32_t length) IRAM_ATTR;
static void* mqtt_is_connected IRAM_ATTR;

static void mqtt_information()
{
    if (mqtt_is_connected == NULL)
        return;

    mqtt_publish(mqtt_prefix(number, "ESP", "SDK Version", 0), system_get_sdk_version(), 0, 0);
    mqtt_publish(mqtt_prefix(number, "ESP", "CPU Frequency", 0), itoa(system_get_cpu_freq(), number + 64, 10), 0, 0);

    mqtt_publish(mqtt_prefix(number, "ESP", "Build", 0), build_date, 0, 0);
    mqtt_publish(mqtt_prefix(number, "ESP", "Version", 0), version, 0, 1);
    struct ip_info ip_info = {};
    if (wifi_get_ip_info(STATION_IF, &ip_info))
    {
        os_sprintf(number + 64, IPSTR, IP2STR(&ip_info.ip));
        mqtt_publish(mqtt_prefix(number, "ESP", "IP", 0), number + 64, 0, 1);
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
        mqtt_publish(mqtt_prefix(number, "ESP", "ResetReason", 0), reason, 0, 0);

        if (info->reason >= REASON_WDT_RST && info->reason <= REASON_SOFT_WDT_RST)
        {
            char buff[256];
            os_sprintf(buff, "Exception:%d flag:%d epc1:0x%08x epc2:0x%08x epc3:0x%08x excvaddr:0x%08x depc:0x%08x", info->exccause, info->reason, info->epc1, info->epc2, info->epc3, info->excvaddr, info->depc);
            mqtt_publish(mqtt_prefix(number, "ESP", "ResetInfo", 0), buff, 0, 0);
        }
        else
        {
            mqtt_publish(mqtt_prefix(number, "ESP", "ResetInfo", 0), reason, 0, 0);
        }
    }
}

static void mqtt_loop()
{
    if (mqtt_is_connected == NULL)
        return;

    // Time
    static uint32 now_timestamp IRAM_ATTR = 0;
    uint32 timestamp = sntp_get_current_timestamp();
    if (now_timestamp != timestamp / 60)
    {
        now_timestamp = timestamp / 60;
        os_sprintf(number + 64, "%d:%d", timestamp / 3600 % 24, timestamp / 60 % 60);
        mqtt_publish(mqtt_prefix(number, "ESP", "Time", 0), number + 64, 0, 0);
    }

    // Heap
    static int now_free_heap IRAM_ATTR = 0;
    int free_heap = xPortGetFreeHeapSize();
    if (now_free_heap != free_heap)
    {
        now_free_heap = free_heap;
        os_sprintf(number + 64, "%d.%05d", free_heap, xPortGetFreeHeapSize() - xPortGetFreeHeapSizeRegion(1));
        mqtt_publish(mqtt_prefix(number, "ESP", "FreeHeap", 0), number + 64, 0, 0);
    }

    // RSSI
    static int now_rssi IRAM_ATTR = 0;
    int rssi = wifi_station_get_rssi();
    if (now_rssi != rssi)
    {
        now_rssi = rssi;
        mqtt_publish(mqtt_prefix(number, "ESP", "RSSI", 0), itoa(rssi | 0xFFFFFF00, number + 64, 10), 0, 0);
    }
}

char* mqtt_prefix(char* pointer, const char* prefix, ...)
{
    va_list args;
    va_start(args, prefix);
    char* output = pointer;
    pointer += os_sprintf(pointer, "%s", wifi_station_get_hostname());
    pointer += os_sprintf(pointer, "/%s", prefix);
    while (const char* name = va_arg(args, char*))
    {
        pointer += os_sprintf(pointer, "/%s", name);
    }
    va_end(args);
    return output;
}

void mqtt_publish(const char* topic, const void* data, int length, int retain)
{
    if (mqtt_is_connected == NULL)
        return;

    char* temp_topic = NULL;
    char* temp_data = NULL;
    if ((uint32_t)topic >= 0x40000000)
    {
        temp_topic = strdup(topic);
        topic = temp_topic;
    }
    if (length == 0)
    {
        for (char c; (c = pgm_read_byte((char*)data + length)); ++length);
    }
    if ((uint32_t)data >= 0x40000000)
    {
        temp_data = (char*)os_malloc(length + 1);
        for (int i = 0; i < length; ++i)
        {
            temp_data[i] = pgm_read_byte((char*)data + i);
        }
        temp_data[length] = 0;
        data = temp_data;
    }
    MQTT_Publish(mqtt_client, topic, (char*)data, length, 0, retain);
    os_free(temp_topic);
    os_free(temp_data);
}

void mqtt_receive(void (*callback)(const char* topic, uint32_t topic_len, const char* data, uint32_t length))
{
    mqtt_receive_callback = callback;
}

void mqtt_setup(const char* ip, int port)
{
    if (mqtt_client == nullptr)
    {
        mqtt_client = (MQTT_Client*)calloc(1, sizeof(MQTT_Client));
        MQTT_InitConnection(mqtt_client, ip, port, 0);
        MQTT_InitClient(mqtt_client, wifi_station_get_hostname(), 0, 0, 120, 1);
        MQTT_InitLWT(mqtt_client, mqtt_prefix(number, "connected", 0), "false", 0, 1);
        MQTT_OnConnected(mqtt_client, [](uint32_t* args)
        {
            mqtt_is_connected = mqtt_client;
            mqtt_publish(mqtt_prefix(number, "connected", 0), "true", 0, 1);
            MQTT_Subscribe(mqtt_client, mqtt_prefix(number, "set", "#", 0), 0);
            mqtt_information();

            // Loop
            static os_timer_t timer IRAM_ATTR;
            os_timer_setfn(&timer, [](void* arg)
            {
                mqtt_loop();
            }, &timer);
            os_timer_disarm(&timer);
            os_timer_arm(&timer, 10000, true);
        });
        MQTT_OnDisconnected(mqtt_client, [](uint32_t* args)
        {
            mqtt_is_connected = NULL;
        });
        MQTT_OnData(mqtt_client, [](uint32_t* args, const char* topic, uint32_t topic_len, const char* data, uint32_t length)
        {
            if (mqtt_receive_callback)
                mqtt_receive_callback(topic, topic_len, data, length);
        });
    }
    MQTT_Connect(mqtt_client);
}

bool mqtt_connected()
{
    return mqtt_is_connected != NULL;
}
