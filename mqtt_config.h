#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

typedef enum
{
    NO_TLS = 0,                             // 0: disable SSL/TLS, there must be no certificate verify between MQTT server and ESP8266
    TLS_WITHOUT_AUTHENTICATION = 1,         // 1: enable SSL/TLS, but there is no a certificate verify
    ONE_WAY_ANTHENTICATION = 2,             // 2: enable SSL/TLS, ESP8266 would verify the SSL server certificate at the same time
    TWO_WAY_ANTHENTICATION = 3,             // 3: enable SSL/TLS, ESP8266 would verify the SSL server certificate and SSL server would verify ESP8266 certificate
}   TLS_LEVEL;

#define DEFAULT_SECURITY        NO_TLS      // very important: you must config DEFAULT_SECURITY for SSL/TLS
#define MQTT_BUF_SIZE           512
#define MQTT_RECONNECT_TIMEOUT  5           /*second*/
#define QUEUE_BUFFER_SIZE       1024
#define PROTOCOL_NAMEv31                    /*MQTT version 3.1 compatible with Mosquitto v0.15*/

#endif // __MQTT_CONFIG_H__
