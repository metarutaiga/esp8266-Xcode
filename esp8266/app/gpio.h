#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266/gpio_register.h>

#undef GPIO_PIN_ADDR
#undef GPIO_EN_OUTPUT
#undef GPIO_DIS_OUTPUT
#undef GPIO_INPUT_GET
#undef GPIO_OUTPUT_SET
#define GPIO_PIN_ADDR(gpio)         (GPIO_PIN0_ADDRESS + gpio * 4)
#define GPIO_EN_OUTPUT(gpio)        GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, BIT(gpio))
#define GPIO_DIS_OUTPUT(gpio)       GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, BIT(gpio))
#define GPIO_INPUT_GET(gpio)        ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> gpio) & BIT0)
#define GPIO_OUTPUT_SET(gpio, set)  \
{ \
    uint32_t address = (set) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS; \
    GPIO_REG_WRITE(address, BIT(gpio)); \
}
#define PIN_PULLDWN_DIS(PIN_NAME)   CLEAR_PERI_REG_MASK(PIN_NAME, PERIPHS_IO_MUX_PULLDWN)
#define PIN_PULLDWN_EN(PIN_NAME)    SET_PERI_REG_MASK(PIN_NAME, PERIPHS_IO_MUX_PULLDWN)

void gpio_init();
void gpio_regist(int gpio, int type, void (*handler)(void* arg, int down, uint32_t cycle), void* arg);
void gpio_pullup(int gpio, bool pullup);
void gpio_pulldown(int gpio, bool pulldown);

#ifdef __cplusplus
}
#endif
