#include "esp8266.h"
#pragma clang diagnostic ignored "-Waddress-of-temporary"
#pragma clang diagnostic ignored "-Winitializer-overrides"
#pragma clang diagnostic ignored "-Wreorder-init-list"
#include <homekit/characteristics.h>
#include <homekit/homekit.h>

static homekit_accessory_t* accessories[] = {
    HOMEKIT_ACCESSORY(
        .id = 1,
        .category = homekit_accessory_category_thermostat,
        .services = (homekit_service_t*[]) {
            HOMEKIT_SERVICE(ACCESSORY_INFORMATION,
                .characteristics = (homekit_characteristic_t*[]) {
                    HOMEKIT_CHARACTERISTIC(IDENTIFY, NULL),
                    HOMEKIT_CHARACTERISTIC(MANUFACTURER, "ESP8266"),
                    HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
                    HOMEKIT_CHARACTERISTIC(NAME, "ESP8266"),
                    HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "123-45-678"),
                    HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, (char*)version),
                    NULL
                }
            )
        }),
    NULL
};

static homekit_server_config_t config =
{
    .accessories = accessories,
    .password = "123-456-789",
};

void homekit_init()
{
    homekit_server_init(&config);
}

void homekit_deinit()
{
    
}
