#include "eagle.h"
#include "gpio.h"
#include "uart.h"

struct uart_context
{
    int bit;
    int head;
    int tail;
    int rx;
    int tx;
    int parity;
    int stop;
    int frame;
    uint32_t baud_cycle;
    uint32_t last_cycle;
    int buffer_size;
    uint8_t buffer[1];
};

static void IRAM_ATTR uart_rx(void* arg, int down, uint32_t cycle)
{
    struct uart_context* context = arg;

    int last_cycle;
    int current_cycle = cycle - context->last_cycle;
    int previous_cycle = current_cycle - context->baud_cycle;
    for (last_cycle = 0; last_cycle < current_cycle; last_cycle += context->baud_cycle)
    {
        if (context->bit == -1)
        {
            context->bit = down ? -1 : 0;
            context->last_cycle = cycle + context->baud_cycle / 2;
            context->buffer[context->head] = 0xFF;
            return;
        }
        if (context->bit < 8)
        {
            int space;
            if (last_cycle < previous_cycle)
            {
                space = down;
            }
            else
            {
                space = down ^ 1;
            }
            if (space)
            {
                context->buffer[context->head] ^= BIT(context->bit);
            }
        }
        context->bit++;
        if (context->bit >= context->frame)
        {
            context->bit = -1;
            context->head++;
            if (context->head >= context->buffer_size)
            {
                context->head = 0;
            }
        }
    }
    context->last_cycle += last_cycle;
}

#ifdef DEMO
static void uart_debug(TaskHandle_t timer)
{
    struct uart_context* context = pvTimerGetTimerID(timer);

    char buffer[64];
    int length = uart_recv(context, buffer, 64);
    if (length <= 0) {
        printf("%d->%d %d\n", context->tail, context->head, esp_get_cycle_count());
        return;
    }

    for (int i = 0; i < length; ++i)
    {
        printf("%02x", buffer[i]);
    }
    printf("\n");
}
#endif

void* uart_init(int rx, int tx, int baud, int data, int parity, int stop, int buffer_size)
{
    struct uart_context* context = calloc(1, sizeof(struct uart_context) + buffer_size - 1);
    context->bit = -1;
    context->head = 0;
    context->tail = 0;
    context->rx = rx;
    context->tx = tx;
    context->parity = parity;
    context->stop = stop;
    context->frame = data + (parity == 'O' || parity == 'E' ? 1 : 0) + stop;
    context->baud_cycle = (esp_clk_cpu_freq() + baud / 2) / baud;
    context->last_cycle = esp_get_cycle_count() - context->baud_cycle / 2;
    context->buffer_size = buffer_size;

    gpio_regist(rx, uart_rx, context);
    gpio_regist(tx, NULL, NULL);
    GPIO_DIS_OUTPUT(rx);
    GPIO_EN_OUTPUT(tx);
    gpio_pullup(rx, true);
    gpio_pullup(tx, true);

#ifdef DEMO
    TimerHandle_t timer = xTimerCreate("uart", 1000 / portTICK_PERIOD_MS, pdTRUE, context, uart_debug);
    xTimerStart(timer, 0);
#endif
    return context;
}

static void uart_wait_until(int begin, int cycle)
{
    while (esp_get_cycle_count() - begin < cycle)
    {
        portYIELD();
    }
}

int uart_send(void* uart, const void* buffer, int length, bool disable_interrupt)
{
    struct uart_context* context = uart;

    if (disable_interrupt)
    {
        vPortETSIntrLock();
    }

    int begin = esp_get_cycle_count();
    int cycle = 0;
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
            GPIO_OUTPUT_SET(context->tx, (__builtin_popcount(c) & 1));
            uart_wait_until(begin, cycle += context->baud_cycle);
            break;
        case 'O':
            GPIO_OUTPUT_SET(context->tx, (__builtin_popcount(c) & 1) ^ 1);
            uart_wait_until(begin, cycle += context->baud_cycle);
            break;
        }
        for (int i = 0; i < context->stop; ++i)
        {
            GPIO_OUTPUT_SET(context->tx, 1);
            uart_wait_until(begin, cycle += context->baud_cycle);
        }
        uart_wait_until(begin, cycle += context->baud_cycle / 2);
    }

    if (disable_interrupt)
    {
        vPortETSIntrUnlock();
    }

    return length;
}

int uart_recv(void* uart, void* buffer, int length)
{
    struct uart_context* context = uart;

    if (context->bit != -1)
    {
        int bit_count = 0;

        uint32_t cycle = esp_get_cycle_count();
        int last_cycle;
        int current_cycle = cycle - context->last_cycle - context->baud_cycle * 16;
        for (last_cycle = 0; last_cycle < current_cycle; last_cycle += context->baud_cycle)
        {
            bit_count++;
            if ((context->bit + bit_count) >= context->frame)
            {
                context->bit = -1;
                context->head++;
                if (context->head >= context->buffer_size)
                {
                    context->head = 0;
                }
                break;
            }
        }
    }

    int available;
    if (context->head >= context->tail)
    {
        available = context->head - context->tail;
    }
    else
    {
        available = context->head + context->buffer_size - context->tail;
    }

    int recv = 0;
    if (buffer)
    {
        recv = length < available ? length : available;
        if (recv)
        {
            char* output = buffer;
            for (int i = 0; i < recv; ++i)
            {
                output[i] = context->buffer[context->tail];
                context->tail++;
                if (context->tail >= context->buffer_size)
                {
                    context->tail = 0;
                }
            }
        }
    }
    else
    {
        recv = available;
    }

    return recv;
}

void uart_reset(void* uart)
{
    struct uart_context* context = uart;

    context->bit = -1;
    context->head = 0;
    context->tail = 0;
    context->last_cycle = esp_get_cycle_count() - context->baud_cycle / 2;
}
