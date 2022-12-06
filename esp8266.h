#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "c_types.h"
#include "osapi.h"
#include "sntp.h"
#include "user_interface.h"
#define BOOL bool
#define uint8_t char
#include "mqtt.h"
#undef uint8_t
#define MQTT_Publish(client, topic, data, data_length, qos, retain) \
    MQTT_Publish(client, topic, data, data_length ? data_length : strlen_P(data), qos, retain)

void setup(void);
void loop(void);
void delay(unsigned int ms);

#ifdef __cplusplus
};
#endif

#include <stdarg.h>
