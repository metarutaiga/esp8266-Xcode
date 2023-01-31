#include "eagle.h"
#include <esp_private/wifi.h>
#define USE_ESP_MQTT 0
#if USE_ESP_MQTT
#include <mqtt_client.h>
#else
#include <sys/socket.h>
#define mqtt_connected mqtt_connected_unused
#define mqtt_publish mqtt_publish_unused
#include <freertos-mqtt/mqtt-service.h>
#undef mqtt_connected
#undef mqtt_publish
#endif
#include "mqtt.h"

#define TAG __FILE_NAME__

#if USE_ESP_MQTT
static esp_mqtt_client_handle_t mqtt_client IRAM_BSS_ATTR;
#else
static void* mqtt_client IRAM_BSS_ATTR;
#endif
static void (*mqtt_receive_callback)(const char* topic, uint32_t topic_len, const char* data, uint32_t length) IRAM_BSS_ATTR;
static bool mqtt_is_connected;

static void mqtt_information()
{
    if (mqtt_is_connected == false)
        return;

    mqtt_publish(mqtt_prefix(number, "ESP", "SDK Version", 0), esp_get_idf_version(), 0, 0);
    mqtt_publish(mqtt_prefix(number, "ESP", "CPU Frequency", 0), itoa(esp_clk_cpu_freq() / FREQ_1MHZ, number + 64, 10), 0, 0);

    mqtt_publish(mqtt_prefix(number, "ESP", "Build", 0), build_date, 0, 0);
    mqtt_publish(mqtt_prefix(number, "ESP", "Version", 0), version, 0, 1);

    tcpip_adapter_ip_info_t ip_info = {};
    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info) == ESP_OK)
    {
        sprintf(number + 64, IPSTR, IP2STR(&ip_info.ip));
        mqtt_publish(mqtt_prefix(number, "ESP", "IP", 0), number + 64, 0, 1);
    }

    const char* reason = NULL;
    switch (esp_reset_reason())
    {
        default:
        case ESP_RST_UNKNOWN:        reason = "Reset reason can not be determined";                     break;
        case ESP_RST_POWERON:        reason = "Reset due to power-on event";                            break;
        case ESP_RST_EXT:            reason = "Reset by external pin (not applicable for ESP8266)";     break;
        case ESP_RST_SW:             reason = "Software reset via esp_restart";                         break;
        case ESP_RST_PANIC:          reason = "Software reset due to exception/panic";                  break;
        case ESP_RST_INT_WDT:        reason = "Reset (software or hardware) due to interrupt watchdog"; break;
        case ESP_RST_TASK_WDT:       reason = "Reset due to task watchdog";                             break;
        case ESP_RST_WDT:            reason = "Reset due to other watchdogs";                           break;
        case ESP_RST_DEEPSLEEP:      reason = "Reset after exiting deep sleep mode";                    break;
        case ESP_RST_BROWNOUT:       reason = "Brownout reset (software or hardware)";                  break;
        case ESP_RST_SDIO:           reason = "Reset over SDIO";                                        break;
        case ESP_RST_FAST_SW:        reason = "Fast reboot";                                            break;
    }
    mqtt_publish(mqtt_prefix(number, "ESP", "ResetReason", 0), reason, 0, 0);
}

static void mqtt_loop(TimerHandle_t xTimer)
{
    if (mqtt_is_connected == false)
        return;

    // Time
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    static uint32_t now_timestamp IRAM_BSS_ATTR;
    uint32_t timestamp = timeinfo.tm_min;
    if (now_timestamp != timestamp)
    {
        now_timestamp = timestamp;
        sprintf(number + 64, "%d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        mqtt_publish(mqtt_prefix(number, "ESP", "Time", 0), number + 64, 0, 0);
    }

    // Heap
    extern size_t heap_caps_get_free_size(uint32_t caps);
    static int now_free_heap IRAM_BSS_ATTR;
    int free_heap = heap_caps_get_free_size(0);
    if (now_free_heap != free_heap)
    {
        now_free_heap = free_heap;
        sprintf(number + 64, "%d.%05d", free_heap, heap_caps_get_free_size(4));
        mqtt_publish(mqtt_prefix(number, "ESP", "FreeHeap", 0), number + 64, 0, 0);
    }

    // RSSI
    static int now_rssi IRAM_BSS_ATTR;
    int rssi = esp_wifi_get_ap_rssi();
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
    pointer += sprintf(pointer, "%s", thisname);
    pointer += sprintf(pointer, "/%s", prefix);
    const char* name;
    while ((name = va_arg(args, char*)))
    {
        pointer += sprintf(pointer, "/%s", name);
    }
    va_end(args);
    return output;
}

void mqtt_publish(const char* topic, const void* data, int length, int retain)
{
    if (mqtt_is_connected == false)
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
        temp_data = (char*)malloc(length + 1);
        for (int i = 0; i < length; ++i)
        {
            temp_data[i] = pgm_read_byte((char*)data + i);
        }
        temp_data[length] = 0;
        data = temp_data;
    }
#if USE_ESP_MQTT
    esp_mqtt_client_publish(mqtt_client, topic, (char*)data, length, 0, retain);
#else
    mqtt_publish_with_length(topic, (char*)data, length, 0, retain);
#endif
    free(temp_topic);
    free(temp_data);
}

void mqtt_receive(void (*callback)(const char* topic, uint32_t topic_len, const char* data, uint32_t length))
{
    mqtt_receive_callback = callback;
}

#if USE_ESP_MQTT
static void mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_is_connected = client;
        esp_mqtt_client_publish(client, mqtt_prefix(number, "connected", 0), "true", 4, 0, 1);
        esp_mqtt_client_subscribe(client, mqtt_prefix(number, "set", "#", 0), 0);
        mqtt_information();
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_is_connected = NULL;
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        if (mqtt_receive_callback)
            mqtt_receive_callback(event->topic, event->topic_len, event->data, event->data_len);
        break;
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb((esp_mqtt_event_handle_t)event_data);
}
#else
static void mqtt_event_handler(mqtt_event_data_t* event)
{
    switch (event->type)
    {
    case MQTT_EVENT_TYPE_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_TYPE_CONNECTED");
        mqtt_is_connected = true;
        mqtt_publish(mqtt_prefix(number, "connected", 0), "true", 0, 1);
        mqtt_subscribe(mqtt_prefix(number, "set", "#", 0));
        mqtt_information();
        break;
    case MQTT_EVENT_TYPE_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_TYPE_DISCONNECTED");
        mqtt_is_connected = false;
        break;
    case MQTT_EVENT_TYPE_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_TYPE_SUBSCRIBED");
        break;
    case MQTT_EVENT_TYPE_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_TYPE_UNSUBSCRIBED");
        break;
    case MQTT_EVENT_TYPE_PUBLISH:
        ESP_LOGI(TAG, "MQTT_EVENT_TYPE_PUBLISH");
        if (mqtt_receive_callback)
            mqtt_receive_callback(event->topic, event->topic_length, event->data, event->data_length);
        break;
    case MQTT_EVENT_TYPE_PUBLISHED:
        ESP_LOGD(TAG, "MQTT_EVENT_TYPE_PUBLISHED");
        break;
    case MQTT_EVENT_TYPE_EXITED:
        ESP_LOGI(TAG, "MQTT_EVENT_TYPE_EXITED");
        break;
    case MQTT_EVENT_TYPE_PUBLISH_CONTINUATION:
        ESP_LOGI(TAG, "MQTT_EVENT_TYPE_PUBLISH_CONTINUATION");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->type);
        break;
    }
}
#endif

void mqtt_setup(const char* ip, int port)
{
    if (mqtt_client == NULL)
    {
#if USE_ESP_MQTT
        esp_mqtt_client_config_t mqtt_cfg =
        {
            .host = ip,
            .port = (uint32_t)port,
            .client_id = thisname,
            .lwt_topic = mqtt_prefix(number, "connected", 0),
            .lwt_msg = "false",
            .lwt_retain = 1,
        };

        mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(mqtt_client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_client);
        esp_mqtt_client_start(mqtt_client);
#else
        ip_addr_t ip_addr;
        inet_pton(AF_INET, ip, &ip_addr);

        static mqtt_connect_info_t info IRAM_BSS_ATTR;
        info.client_id = thisname;
        info.keepalive = 60;
        info.will_topic = strdup(mqtt_prefix(number, "connected", 0));
        info.will_message = "false";
        info.will_retain = 1;

        ESP_LOGI(TAG, "%s", info.will_topic);
        mqtt_init(malloc(512), 512, malloc(512), 512);
        mqtt_connect(&ip_addr, port, 1, &info, mqtt_event_handler);
#endif
        static TimerHandle_t timer IRAM_BSS_ATTR;
        if (timer == NULL)
        {
            timer = xTimerCreate("MQTT Timer", 10000 / portTICK_PERIOD_MS, pdTRUE, mqtt_client, mqtt_loop);
        }
        xTimerStart(timer, 0);
    }
}

bool mqtt_connected()
{
    return mqtt_is_connected;
}
