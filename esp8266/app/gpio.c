#include "esp8266.h"

struct gpio_handler
{
    void (*handler)(void* arg, int up, uint32_t cycle);
    void* arg;
};
static struct gpio_handler handlers[16] IRAM_ATTR;

static void IRAM_FLASH_ATTR gpio_handler(void* arg)
{
    uint8_t i;
    uint32_t cycle = esp_get_cycle_count();
    uint32_t gpio_in = GPIO_REG_READ(GPIO_IN_ADDRESS);
    uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    while ((i = __builtin_ffs(gpio_status)))
    {
        i--;
        gpio_status &= ~BIT(i);
        if (handlers[i].handler)
        {
            handlers[i].handler(handlers[i].arg, (gpio_in >> i) & BIT0, cycle);
        }
    }
}

static const uint32_t gpio_address[16] =
{
    PERIPHS_IO_MUX_GPIO0_U,
    PERIPHS_IO_MUX_U0TXD_U,
    PERIPHS_IO_MUX_GPIO2_U,
    PERIPHS_IO_MUX_U0RXD_U,
    PERIPHS_IO_MUX_GPIO4_U,
    PERIPHS_IO_MUX_GPIO5_U,
    0, // PERIPHS_IO_MUX_SD_CLK_U,
    0, // PERIPHS_IO_MUX_SD_DATA0_U,
    0, // PERIPHS_IO_MUX_SD_DATA1_U,
    0, // PERIPHS_IO_MUX_SD_DATA2_U,
    0, // PERIPHS_IO_MUX_SD_DATA3_U,
    0, // PERIPHS_IO_MUX_SD_CMD_U,
    PERIPHS_IO_MUX_MTDI_U,
    PERIPHS_IO_MUX_MTCK_U,
    PERIPHS_IO_MUX_MTMS_U,
    PERIPHS_IO_MUX_MTDO_U,
};

void gpio_regist(int gpio, void (*handler)(void* arg, int up, uint32_t cycle), void* arg)
{
    if (gpio < 0 || gpio > 16)
        return;

    switch (gpio)
    {
    case 0:
    case 2:
    case 4:
    case 5:
        PIN_FUNC_SELECT(gpio_address[gpio], 0);
        break;
    case 1:
    case 3:
    case 12:
    case 13:
    case 14:
    case 15:
        PIN_FUNC_SELECT(gpio_address[gpio], 3);
        break;
    default:
        return;
    }

    handlers[gpio].handler = handler;
    handlers[gpio].arg = arg;
    ETS_GPIO_INTR_DISABLE();
    ETS_GPIO_INTR_ATTACH(gpio_handler, NULL);
    ETS_GPIO_INTR_ENABLE();

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
}

void gpio_pullup(int gpio, bool pullup)
{
    if (gpio < 0 || gpio > 16)
        return;

    uint32_t mux = gpio_address[gpio];
    if (mux == 0)
        return;

    if (pullup)
    {
        PIN_PULLUP_EN(mux);
    }
    else
    {
        PIN_PULLUP_DIS(mux);
    }
}

void gpio_pulldown(int gpio, bool pulldown)
{
    if (gpio < 0 || gpio > 16)
        return;

    uint32_t mux = gpio_address[gpio];
    if (mux == 0)
        return;

    if (pulldown)
    {
        PIN_PULLDWN_EN(mux);
    }
    else
    {
        PIN_PULLDWN_DIS(mux);
    }
}
