#include "esp8266.h"
#include "../driver_lib/include/driver/uart_register.h"
#include "gpio.h"

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

static void IRAM_FLASH_ATTR uart_rx(void* arg, int up, uint32_t cycle)
{
    struct uart_context* context = arg;

    int last_cycle;
    int current_cycle = cycle - context->last_cycle;
    int previous_cycle = current_cycle - context->baud_cycle;
    for (last_cycle = 0; last_cycle < current_cycle; last_cycle += context->baud_cycle)
    {
        if (context->bit == -1)
        {
            context->bit = 0;
            context->last_cycle = cycle + context->baud_cycle / 2;
            context->buffer[context->head] = 0xFF;
            return;
        }
        if (context->bit < 8)
        {
            int space;
            if (last_cycle < previous_cycle)
            {
                space = up;
            }
            else
            {
                space = up ^ 1;
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

void* uart_init(int rx, int tx, int baud, int data, int parity, int stop, int buffer_size)
{
    struct uart_context* context = os_zalloc(sizeof(struct uart_context) + buffer_size - 1);
    context->bit = -1;
    context->head = 0;
    context->tail = 0;
    context->rx = rx;
    context->tx = tx;
    context->parity = parity;
    context->stop = stop;
    context->frame = data + (parity == 'O' || parity == 'E' ? 1 : 0) + stop;
    context->baud_cycle = (system_get_cpu_freq() * 1000 * 1000 + baud / 2) / baud;
    context->last_cycle = esp_get_cycle_count() - context->baud_cycle / 2;
    context->buffer_size = buffer_size;

    gpio_regist(rx, uart_rx, context);
    gpio_regist(tx, NULL, NULL);
    GPIO_DIS_OUTPUT(rx);
    GPIO_EN_OUTPUT(tx);
    gpio_pullup(rx, true);
    gpio_pullup(tx, true);
    gpio_pulldown(rx, false);
    gpio_pulldown(tx, false);

    if (rx == -13)
    {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 4);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 4);
        SET_PERI_REG_MASK(0x3FF00028, BIT2);
        GPIO_DIS_OUTPUT(13);
        uart_div_modify(0, UART_CLK_FREQ / baud);
        uint32_t mode = 0;
        mode |= ((3) & UART_BIT_NUM) << UART_BIT_NUM_S;
        mode |= ((parity != 'O' ? 0 : 1) & UART_PARITY_M) << UART_PARITY_S;
        mode |= ((parity == 0 ? 0 : 1) & UART_PARITY_EN_M) << UART_PARITY_EN_S;
        mode |= ((stop == 2 ? 3 : 1) & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S;
        WRITE_PERI_REG(UART_CONF0(0), mode);
    }
    else if (rx >= 0)
    {
        GPIO_DIS_OUTPUT(rx);
    }

    if (tx == -2)
    {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
        GPIO_EN_OUTPUT(2);
        uart_div_modify(1, UART_CLK_FREQ / baud);
        uint32_t mode = 0;
        mode |= ((3) & UART_BIT_NUM) << UART_BIT_NUM_S;
        mode |= ((parity != 'O' ? 0 : 1) & UART_PARITY_M) << UART_PARITY_S;
        mode |= ((parity == 0 ? 0 : 1) & UART_PARITY_EN_M) << UART_PARITY_EN_S;
        mode |= ((stop == 2 ? 3 : 1) & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S;
        WRITE_PERI_REG(UART_CONF0(1), mode);
    }
    else if (tx >= 0)
    {
        GPIO_EN_OUTPUT(tx);
    }

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

    if (context->tx == -2)
    {
        const char* text = buffer;
        for (int i = 0; i < length; ++i)
        {
            char c = text[i];
            while ((READ_PERI_REG(UART_STATUS(1)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S)) >= 127);
            WRITE_PERI_REG(UART_FIFO(1), c);
        }
        return length;
    }

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
            GPIO_OUTPUT_SET(context->tx, __builtin_popcount(c) & 1);
            break;
        case 'O':
            uart_wait_until(begin, cycle += context->baud_cycle);
            GPIO_OUTPUT_SET(context->tx, __builtin_popcount(c) & 1 ^ 1);
            break;
        }
        for (int i = 0; i < context->stop; ++i)
        {
            uart_wait_until(begin, cycle += context->baud_cycle);
            GPIO_OUTPUT_SET(context->tx, 1);
        }
        uart_wait_until(begin, cycle += context->baud_cycle + context->baud_cycle / 2);
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
                if ((READ_PERI_REG(UART_STATUS(0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) == 0)
                {
                    recv = i;
                    break;
                }
                char c = READ_PERI_REG(UART_FIFO(0));
                text[i] = c;
            }
        }
        else
        {
            recv = (READ_PERI_REG(UART_STATUS(0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S));
        }

        return recv;
    }

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
