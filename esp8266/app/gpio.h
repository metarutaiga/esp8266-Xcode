#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void gpio_regist(int gpio, void (*handler)(void* arg, int up), void* arg);

#ifdef __cplusplus
}
#endif
