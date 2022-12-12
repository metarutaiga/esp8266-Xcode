#pragma once

#define IRAM_ATTR __attribute__((section(".iram.text")))

#ifdef __cplusplus
extern "C" {
#endif

#include "c_types.h"
#include "osapi.h"
#include "sntp.h"
#include "user_interface.h"

void setup(void);
void loop(void);
void delay(unsigned int ms);

#ifdef __cplusplus
};
#endif

#include <stdarg.h>
