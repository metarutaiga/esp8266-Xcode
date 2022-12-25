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

extern void rijndaelKeySetupEnc(u32 rk[], const u8 cipherKey[]);
extern void system_restart_local();
inline uint32_t IRAM_ATTR esp_get_cycle_count()
{
    uint32_t ccount;
    __asm__ __volatile__("rsr %0,ccount":"=a"(ccount));
    return ccount;
}

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
