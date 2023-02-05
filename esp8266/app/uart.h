#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* uart_init(int rx, int tx, int baud, int data, int parity, int stop, int buffer_size);
int uart_send(void* uart, const void* buffer, int length);
int uart_recv(void* uart, void* buffer, int length);
void uart_reset(void* uart);

#ifdef __cplusplus
}
#endif
