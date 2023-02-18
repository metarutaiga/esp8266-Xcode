#include "eagle.h"
#include <driver/gpio.h>
#include <esp8266/uart_struct.h>
#include <esp8266/uart_register.h>
#include "gpio.h"
#include "uart.h"

#define UART_RX_ONE_BIT 0
#define UART_RX_FULL 1

#define TAG __FILE_NAME__

struct uart_context
{
    int head;
    int tail;
    int rx;
    int tx;
    int parity;
    int stop;
    int frame;
    uint32_t baud_cycle;
#if UART_RX_ONE_BIT
    int bit;
    uint32_t last_cycle;
#endif
    int buffer_size;
    uint8_t buffer[1];
};

static inline void uart_wait_until(int begin, int cycle)
{
    while (esp_get_cycle_count() - begin < cycle)
    {
//      portYIELD();
    }
}

static void IRAM_ATTR uart_rx(void* arg, int down, uint32_t cycle)
{
    struct uart_context* context = arg;
#if UART_RX_ONE_BIT
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
#else
    for (int i = 0; i < 256; ++i)
    {
        int begin = esp_get_cycle_count();
        int cycle = 0;
        for (int i = 0; i < 8; ++i)
        {
            if (GPIO_INPUT_GET(context->rx) == 0)
            {
                uint8_t c = 0;
                for (int i = 0; i < 8; ++i)
                {
                    uart_wait_until(begin, cycle += context->baud_cycle);
                    c |= GPIO_INPUT_GET(context->rx) << i;
                }
                switch (context->parity)
                {
                default:
                    break;
                case 'E':
                    uart_wait_until(begin, cycle += context->baud_cycle);
                    break;
                case 'O':
                    uart_wait_until(begin, cycle += context->baud_cycle);
                    break;
                }
                uart_wait_until(begin, cycle += context->baud_cycle);
                for (int i = 0; i < 8; ++i)
                {
                    if (GPIO_INPUT_GET(context->rx) == 1)
                    {
                        context->buffer[context->head] = c;
                        context->head++;
                        if (context->head >= context->buffer_size)
                        {
                            context->head = 0;
                        }
                        goto stop;
                    }
                    uart_wait_until(begin, cycle += context->baud_cycle / 8);
                }
                goto stop;
            }
            uart_wait_until(begin, cycle += context->baud_cycle / 8);
        }

stop:
#if UART_RX_FULL
        for (int i = 1; i < context->stop; ++i)
        {
            uart_wait_until(begin, cycle += context->baud_cycle);
        }
        for (int i = 0; i < 64; ++i)
        {
            if (GPIO_INPUT_GET(context->rx) == 0)
            {
                goto next;
            }
            uart_wait_until(begin, cycle += context->baud_cycle / 8);
        }
#endif
        return;
next:
        continue;
    }
#endif
}

#ifdef DEMO
static void uart_debug(TaskHandle_t timer)
{
    struct uart_context* context = pvTimerGetTimerID(timer);

    char buffer[128];
    int length = uart_recv(context, buffer, 128);
    if (length > 0)
    {
        char* output = malloc(length * 2 + 16);
        for (int i = 0; i < length; ++i)
        {
            sprintf(output + i * 2, "%02x", buffer[i]);
        }
        ESP_LOGI(TAG, "%d:%s", length, output);
        free(output);
    }
}
#endif

void* uart_init(int rx, int tx, int baud, int data, int parity, int stop, int buffer_size)
{
    struct uart_context* context = calloc(1, sizeof(struct uart_context) + buffer_size - 1);

    context->head = 0;
    context->tail = 0;
    context->rx = rx;
    context->tx = tx;
    context->parity = parity;
    context->stop = stop;
    context->frame = data + (parity == 'O' || parity == 'E' ? 1 : 0) + stop;
    context->baud_cycle = (esp_clk_cpu_freq() + baud / 2) / baud;
#if UART_RX_ONE_BIT
    context->bit = -1;
    context->last_cycle = esp_get_cycle_count() - context->baud_cycle / 2;
#endif
    context->buffer_size = buffer_size;
#if UART_RX_ONE_BIT
    gpio_regist(rx, GPIO_INTR_ANYEDGE, uart_rx, context);
#else
    gpio_regist(rx, GPIO_INTR_NEGEDGE, uart_rx, context);
#endif
    gpio_regist(tx, 0, NULL, NULL);
    gpio_pullup(rx, true);
    gpio_pullup(tx, true);
    gpio_pulldown(rx, false);
    gpio_pulldown(tx, false);

    if (rx == -13)
    {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_UART0_CTS);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_UART0_RTS);
        SET_PERI_REG_MASK(UART_SWAP_REG, BIT2);
        uart0.clk_div.div_int = UART_CLK_FREQ / baud;
        uart0.conf0.bit_num = 3;
        uart0.conf0.parity = parity != 'O' ? 0 : 1;
        uart0.conf0.parity_en = parity == 0 ? 0 : 1;
        uart0.conf0.stop_bit_num = stop == 2 ? 3 : 1;
        uart0.conf1.rx_flow_en = 0;
        uart0.conf1.rx_tout_en = 0;
        uart0.conf0.rxfifo_rst = 1;
        uart0.conf0.rxfifo_rst = 0;
    }

    if (tx == -2)
    {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_UART1_TXD_BK);
        uart1.clk_div.div_int = UART_CLK_FREQ / baud;
        uart1.conf0.bit_num = 3;
        uart1.conf0.parity = parity != 'O' ? 0 : 1;
        uart1.conf0.parity_en = parity == 0 ? 0 : 1;
        uart1.conf0.stop_bit_num = stop == 2 ? 3 : 1;
        uart1.conf0.tx_flow_en = 0;
        uart1.conf0.txfifo_rst = 1;
        uart1.conf0.txfifo_rst = 0;
    }

#ifdef DEMO
    TimerHandle_t timer = xTimerCreate("uart", 1000 / portTICK_PERIOD_MS, pdTRUE, context, uart_debug);
    xTimerStart(timer, 0);
#endif
    return context;
}

int uart_send(void* uart, const void* buffer, int length, bool disable_interrupt)
{
    struct uart_context* context = uart;

    if (context->tx == -2)
    {
        const char* text = buffer;
        for (int i = 0; i < length; ++i)
        {
            char c = text[i];
            while (uart1.status.txfifo_cnt >= 127);
            uart1.fifo.rw_byte = c;
        }
        return length;
    }

    if (disable_interrupt)
    {
        vPortETSIntrLock();
    }

    GPIO_EN_OUTPUT(context->tx);
    int begin = esp_get_cycle_count();
    int cycle = 0;
    for (int i = 0; i < length; ++i)
    {
        uint8_t c = ((uint8_t*)buffer)[i];
        GPIO_OUTPUT_SET(context->tx, 0);
        for (int i = 0; i < 8; ++i)
        {
            uart_wait_until(begin, cycle += context->baud_cycle);
            GPIO_OUTPUT_SET(context->tx, c & BIT(i) ? 1 : 0);
        }
        switch (context->parity)
        {
        default:
            break;
        case 'E':
            uart_wait_until(begin, cycle += context->baud_cycle);
            GPIO_OUTPUT_SET(context->tx, (__builtin_popcount(c) & 1));
            break;
        case 'O':
            uart_wait_until(begin, cycle += context->baud_cycle);
            GPIO_OUTPUT_SET(context->tx, (__builtin_popcount(c) & 1) ^ 1);
            break;
        }
        for (int i = 0; i < context->stop; ++i)
        {
            uart_wait_until(begin, cycle += context->baud_cycle);
            GPIO_OUTPUT_SET(context->tx, 1);
        }
        uart_wait_until(begin, cycle += context->baud_cycle + context->baud_cycle / 2);
    }
    GPIO_DIS_OUTPUT(context->tx);

    if (disable_interrupt)
    {
        vPortETSIntrUnlock();
    }

    return length;
}

int uart_recv(void* uart, void* buffer, int length)
{
    struct uart_context* context = uart;

    if (context->rx == -13)
    {
        int recv = 0;
        if (buffer)
        {
            char* text = buffer;
            for (int i = 0; i < length; ++i)
            {
                if (uart0.status.rxfifo_cnt == 0)
                {
                    recv = i;
                    break;
                }
                char c = uart0.fifo.rw_byte;
                text[i] = c;
            }
        }
        else
        {
            recv = uart0.status.rxfifo_cnt;
        }

        return recv;
    }
#if UART_RX_ONE_BIT
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
#endif
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

    context->head = 0;
    context->tail = 0;
#if UART_RX_ONE_BIT
    context->bit = -1;
    context->last_cycle = esp_get_cycle_count() - context->baud_cycle / 2;
#endif
}
