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
    ETS_GPIO_INTR_DISABLE();
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
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
    ETS_GPIO_INTR_DISABLE();

    switch (gpio)
    {
    case 0:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
        break;
    case 1:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_U0TXD_U);
        break;
    case 2:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
        break;
    case 3:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);
        break;
    case 4:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);
        break;
    case 5:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
        break;
    case 9:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_SD_DATA2_U);
        break;
    case 10:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_SD_DATA3_U);
        break;
    case 12:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDI_U);
        break;
    case 13:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_MTCK_U);
        break;
    case 14:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_MTMS_U);
        break;
    case 15:
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
        PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDO_U);
        break;
    }
    GPIO_DIS_OUTPUT(gpio);

    handlers[gpio].handler = handler;
    handlers[gpio].arg = arg;
    ETS_GPIO_INTR_ATTACH(gpio_handler, NULL);

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio));
    gpio_pin_intr_state_set(gpio, handler ? GPIO_PIN_INTR_ANYEDGE : GPIO_PIN_INTR_DISABLE);

    ETS_GPIO_INTR_ENABLE();
}
