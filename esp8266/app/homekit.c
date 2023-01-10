#include "esp8266.h"
#pragma clang diagnostic ignored "-Winitializer-overrides"
#define HOMEKIT_SHORT_APPLE_UUIDS
#include <homekit/characteristics.h>
#include <homekit/homekit.h>

extern homekit_accessory_t const* const accessories[] __attribute__((weak));
homekit_accessory_t const* const accessories[] =
{
    &(homekit_accessory_t const) HOMEKIT_ACCESSORY_(.id = 1,
                                                    .category = homekit_accessory_category_sensor,
                                                    .services = (homekit_service_t **)(homekit_service_t const* const[])
    {
        &(homekit_service_t const) HOMEKIT_SERVICE_(ACCESSORY_INFORMATION,
                                                    .characteristics = (homekit_characteristic_t **)(homekit_characteristic_t const* const[])
        {
            HOMEKIT_CHARACTERISTIC(IDENTIFY, NULL),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(NAME, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "123-45-678"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, (char*)version),
            NULL
        }),
        &(homekit_service_t const) HOMEKIT_SERVICE_(TEMPERATURE_SENSOR,
                                                    .characteristics = (homekit_characteristic_t **)(homekit_characteristic_t const* const[])
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
