#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <c_types.h>
#include <mem.h>
#include <osapi.h>
#include <sntp.h>
#include <user_interface.h>
#include <espconn.h>

extern const char version[16];
extern char thisname[16];
extern char number[128];

void setup(void);
void loop(void);
void delay(unsigned int ms);

#ifdef __cplusplus
};
#endif

#include <stdarg.h>
