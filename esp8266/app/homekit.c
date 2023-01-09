#include "esp8266.h"
#pragma clang diagnostic ignored "-Winitializer-overrides"
#define HOMEKIT_SHORT_APPLE_UUIDS
#include <homekit/characteristics.h>
#include <homekit/homekit.h>

extern const homekit_accessory_t* accessories[] __attribute__((weak));
const homekit_accessory_t* accessories[] =
{
    HOMEKIT_ACCESSORY(.id = 1,
                      .category = homekit_accessory_category_sensor,
                      .services = (homekit_service_t*[])
    {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[])
        {
            HOMEKIT_CHARACTERISTIC(IDENTIFY, NULL),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(NAME, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "123-45-678"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, (char*)version),
            NULL
        }),
        HOMEKIT_SERVICE(TEMPERATURE_SENSOR, .characteristics = (homekit_characteristic_t*[])
        {
            HOMEKIT_CHARACTERISTIC(NAME, "Temperature"),
            HOMEKIT_CHARACTERISTIC(CURRENT_TEMPERATURE, 0),
            NULL
        }),
        NULL
    }),
    NULL
};

static homekit_server_config_t config =
{
    .accessories = (homekit_accessory_t**)accessories,
    .password = "123-45-678",
};

void homekit_init()
{
    homekit_server_init(&config);
}

void homekit_deinit()
{
    homekit_server_deinit();
}
