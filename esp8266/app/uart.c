#include "esp8266.h"
#include "gpio.h"

#define DIRECT 1
#define GPIO_EN_OUTPUT(gpio)
#if DIRECT
#undef GPIO_EN_OUTPUT
#undef GPIO_DIS_OUTPUT
#undef GPIO_OUTPUT_SET
#define GPIO_EN_OUTPUT(gpio)        GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, BIT(gpio))
#define GPIO_DIS_OUTPUT(gpio)       GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, BIT(gpio))
#define GPIO_OUTPUT_SET(gpio, set)  \
{ \
    uint32_t address = (set) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS; \
    GPIO_REG_WRITE(address, BIT(gpio)); \
}
#endif

struct uart_context
{
    int bit;
    int offset;
    int rx;
    int tx;
    int parity;
    int stop;
    int frame;
    uint32_t baud_cycle;
    uint32_t last_cycle;
    uint8_t buffer[128];
};

static void IRAM_FLASH_ATTR uart_rx(void* arg, int up)
{
    struct uart_context* context = arg;

    uint32_t cycle = esp_get_cycle_count();
    uint32_t delta_cycle = cycle - context->last_cycle;
    if (delta_cycle > context->baud_cycle * 16)
    {
        context->bit = -1;
        context->offset = 0;
        context->last_cycle = cycle - context->baud_cycle / 2;
    }

    int last_cycle;
    int current_cycle = cycle - context->last_cycle;
    int previous_cycle = current_cycle - context->baud_cycle;
    for (last_cycle = 0; last_cycle < current_cycle; last_cycle += context->baud_cycle)
    {
        if (context->bit == -1)
        {
            context->buffer[context->offset] = 0;

            cycle = esp_get_cycle_count();
            context->last_cycle = cycle - context->baud_cycle / 2;
            current_cycle = cycle - context->last_cycle;
            previous_cycle = current_cycle - context->baud_cycle;
        }
        else if (context->bit < 8)
        {
            int space;
            if (last_cycle < previous_cycle)
            {
                space = up ^ BIT0;
            }
            else
            {
                space = up;
            }
            if (space)
            {
                context->buffer[context->offset] |= BIT(context->bit);
            }
        }
        context->bit++;
        if (context->bit >= context->frame)
        {
            context->bit = -1;
            context->offset++;
        }
    }
    context->last_cycle += last_cycle;
}

#ifdef DEMO
static void uart_debug(void* arg)
{
    struct uart_context* context = arg;

    for (int i = 0; i < context->offset; ++i)
    {
        os_printf("%02x", context->buffer[i]);
    }
}
#endif

void* uart_init(int rx, int tx, int baud, int data, int parity, int stop)
{
    struct uart_context* context = os_zalloc(sizeof(struct uart_context));
    context->bit = -1;
    context->offset = 0;
    context->rx = rx;
    context->tx = tx;
    context->parity = parity;
    context->stop = stop;
    context->frame = data + (parity == 'O' || parity == 'E' ? 1 : 0) + stop;
    context->baud_cycle = (system_get_cpu_freq() * 1000 * 1000 + baud / 2) / baud;
    context->last_cycle = esp_get_cycle_count() - context->baud_cycle / 2;

    gpio_regist(rx, uart_rx, context);
    gpio_regist(tx, NULL, NULL);

#ifdef DEMO
    // Debug
    static os_timer_t timer IRAM_ATTR;
    os_timer_setfn(&timer, uart_debug, context);
    os_timer_arm(&timer, 1000, true);
#endif
    return context;
}

static void uart_wait_until(int begin, int cycle)
{
    while (esp_get_cycle_count() - begin < cycle)
    {
        delay(0);
    }
}

int uart_send(void* uart, const void* buffer, int length)
{
    struct uart_context* context = uart;

    int begin = esp_get_cycle_count();
    int cycle = 0;
    GPIO_EN_OUTPUT(context->tx);
    for (int i = 0; i < length; ++i)
    {
        uint8_t c = ((uint8_t*)buffer)[i];
        GPIO_OUTPUT_SET(context->tx, 0);
        uart_wait_until(begin, cycle += context->baud_cycle);
        for (int i = 0; i < 8; ++i)
        {
            GPIO_OUTPUT_SET(context->tx, c & BIT(i) ? 1 : 0);
            uart_wait_until(begin, cycle += context->baud_cycle);
        }
        switch (context->parity)
        {
        default:
            break;
        case 'E':
            GPIO_OUTPUT_SET(context->tx, __builtin_popcount(c) & 1 ^ 1);
            uart_wait_until(begin, cycle += context->baud_cycle);
            break;
        case 'O':
            GPIO_OUTPUT_SET(context->tx, __builtin_popcount(c) & 1);
            uart_wait_until(begin, cycle += context->baud_cycle);
            break;
        }
        for (int i = 0; i < context->stop; ++i)
        {
            GPIO_OUTPUT_SET(context->tx, 1);
            uart_wait_until(begin, cycle += context->baud_cycle);
        }
        uart_wait_until(begin, cycle += context->baud_cycle);
    }
    GPIO_DIS_OUTPUT(context->tx);

    return length;
}

int uart_recv(void* uart, void* buffer, int length)
{
    struct uart_context* context = uart;

    if (buffer)
    {
        length = length < context->offset ? length : context->offset;
        os_memcpy(buffer, context->buffer, length);
        context->offset -= length;
        os_memmove(context->buffer, context->buffer + length, context->offset);
    }
    else
    {
        length = context->offset;
    }
    return length;
}
