#include "esp8266.h"

struct gpio_handler
{
    void (*handler)(void* arg, int up);
    void* arg;
};
static struct gpio_handler handlers[16] IRAM_ATTR;

static void IRAM_FLASH_ATTR gpio_handler(void* arg)
{
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    if (gpio_status == 0)
        return;
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    ETS_GPIO_INTR_DISABLE();
    for (int i = 0; i < 16; ++i)
    {
        if (gpio_status & BIT(i))
        {
            if (handlers[i].handler)
            {
                handlers[i].handler(handlers[i].arg, GPIO_INPUT_GET(i));
            }
        }
    }
    ETS_GPIO_INTR_ENABLE();
}

void gpio_regist(int gpio, void (*handler)(void* arg, int up), void* arg)
{
    if (gpio < 0 || gpio > 16)
        return;

    ETS_GPIO_INTR_DISABLE();

    switch (gpio)
    {
    case 0:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
        break;
    case 1:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
        break;
    case 2:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
        break;
    case 3:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
        break;
    case 4:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
        break;
    case 5:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
        break;
    case 12:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
        break;
    case 13:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
        break;
    case 14:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
        break;
    case 15:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
        break;
    }
    GPIO_DIS_OUTPUT(gpio);

    handlers[gpio].handler = handler;
    handlers[gpio].arg = arg;
    ETS_GPIO_INTR_ATTACH(gpio_handler, NULL);

    uint32_t pin = GPIO_REG_READ(GPIO_PIN_ADDR(gpio));
    pin &= ~GPIO_PIN_WAKEUP_ENABLE_MASK;
    pin &= ~GPIO_PIN_INT_TYPE_MASK;
    pin &= ~GPIO_PIN_PAD_DRIVER_MASK;
    pin &= ~GPIO_PIN_SOURCE_MASK;
    if (handler)
    {
        pin |= GPIO_PIN_WAKEUP_ENABLE_SET(GPIO_WAKEUP_DISABLE & 1);
        pin |= GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_ANYEDGE);
        pin |= GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE & 1);
        pin |= GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE);
    }
    else
    {
        pin |= GPIO_PIN_WAKEUP_ENABLE_SET(GPIO_WAKEUP_DISABLE & 1);
        pin |= GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE);
        pin |= GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE & 1);
        pin |= GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE);
    }
    GPIO_REG_WRITE(GPIO_PIN_ADDR(gpio), pin);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio));

    ETS_GPIO_INTR_ENABLE();
}

void gpio_pullup(int gpio, bool pullup)
{
    if (gpio < 0 || gpio > 16)
        return;

    uint32_t mux;
    switch (gpio)
    {
    case 0:
        mux = PERIPHS_IO_MUX_GPIO0_U;
        break;
    case 1:
        mux = PERIPHS_IO_MUX_U0TXD_U;
        break;
    case 2:
        mux = PERIPHS_IO_MUX_GPIO2_U;
        break;
    case 3:
        mux = PERIPHS_IO_MUX_U0RXD_U;
        break;
    case 4:
        mux = PERIPHS_IO_MUX_GPIO4_U;
        break;
    case 5:
        mux = PERIPHS_IO_MUX_GPIO5_U;
        break;
    case 12:
        mux = PERIPHS_IO_MUX_MTDI_U;
        break;
    case 13:
        mux = PERIPHS_IO_MUX_MTCK_U;
        break;
    case 14:
        mux = PERIPHS_IO_MUX_MTMS_U;
        break;
    case 15:
        mux = PERIPHS_IO_MUX_MTDO_U;
        break;
    default:
        return;
    }

    if (pullup)
    {
        PIN_PULLUP_EN(mux);
    }
    else
    {
        PIN_PULLUP_DIS(mux);
    }
}
