#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void gpio_regist(int gpio, void (*handler)(void* arg, int up, uint32_t cycle), void* arg);
void gpio_pullup(int gpio, bool pullup);
void gpio_pulldown(int gpio, bool pulldown);

#ifdef __cplusplus
}
#endif
