#pragma once

#ifdef __cplusplus
extern "C" {
#endif

char* mqtt_prefix(char* pointer, const char* prefix, ...);
void mqtt_publish(const char* topic, const void* data, int length, int retain);
void mqtt_receive(void (*callback)(const char* topic, uint32_t topic_len, const char* data, uint32_t length));

void mqtt_setup(const char* ip, int port);

#ifdef __cplusplus
}
#endif
