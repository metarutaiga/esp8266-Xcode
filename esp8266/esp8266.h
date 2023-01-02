#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <c_types.h>
#include <mem.h>
#include <osapi.h>
#include <sntp.h>
#include <time.h>
#include <user_interface.h>
#include <espconn.h>

#define DIRECT 1
#define GPIO_EN_OUTPUT(gpio)
#if DIRECT
#undef GPIO_EN_OUTPUT
#undef GPIO_DIS_OUTPUT
#undef GPIO_INPUT_GET
#undef GPIO_OUTPUT_SET
#define GPIO_EN_OUTPUT(gpio)        GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, BIT(gpio))
#define GPIO_DIS_OUTPUT(gpio)       GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, BIT(gpio))
#define GPIO_INPUT_GET(gpio)        ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> gpio) & BIT0)
#define GPIO_OUTPUT_SET(gpio, set)  \
{ \
    uint32_t address = (set) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS; \
    GPIO_REG_WRITE(address, BIT(gpio)); \
}
#endif

extern uint32_t lfs_crc(uint32_t crc, const void* buffer, size_t size);
extern void rijndaelKeySetupEnc(u32 rk[], const u8 cipherKey[]);
extern void system_restart_local();
extern uint64_t system_get_time64();
extern uint32_t system_get_time_ms();
extern struct tm* sntp_localtime(const time_t* tim_p);
inline uint32_t IRAM_FLASH_ATTR esp_get_cycle_count()
{
    uint32_t ccount;
    __asm__ __volatile__("rsr %0,ccount":"=a"(ccount));
    return ccount;
}

extern const char* version;
extern const char* build_date;
extern char number[128];

void setup(void);
void loop(void);
void delay(unsigned int ms);

#ifdef __cplusplus
};
#endif

#include <stdarg.h>
