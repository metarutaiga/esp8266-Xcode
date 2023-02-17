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
#define PIN_PULLDWN_DIS(PIN_NAME)   CLEAR_PERI_REG_MASK(PIN_NAME, PERIPHS_IO_MUX_PULLUP2)
#define PIN_PULLDWN_EN(PIN_NAME)    SET_PERI_REG_MASK(PIN_NAME, PERIPHS_IO_MUX_PULLUP2)
#endif

extern void ets_write_char(char c);
extern uint32_t lfs_crc(uint32_t crc, const void* buffer, size_t size);
extern int system_station_got_ip_set(ip_addr_t* ip, ip_addr_t* mask, ip_addr_t* gw);
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
extern size_t xPortGetFreeHeapSize(void);
extern size_t xPortGetFreeHeapSizeRegion(int region);

extern const char version[];
extern const char build_date[];
extern char number[128];

void setup(void);
void loop(void);
void delay(unsigned int ms);

#ifdef __cplusplus
};
#endif

#include <stdarg.h>

#ifdef __cplusplus
#include <string>
#if 0
typedef std::string string;
#else
class string : public std::string
{
public:
    using std::string::string;
    string(std::string&& __str) : std::string(std::move(__str)) {}
    string(const std::string& __str) : std::string(__str) {}
    string(const char* __s) { append(__s); }
    using std::string::append;
    string& append(const char* __s)
    {
        for (uint32_t i = (uint32_t)__s % 4; i > 0 && i < 4; i += 1)
        {
            char c = pgm_read_byte(__s);
            if (c == 0)
                return *this;
            std::string::push_back(c);
            __s += 1;
        }
        for (;;)
        {
            uint32_t d = pgm_read_dword_aligned(__s);
            for (uint32_t i = 0; i < 4; i += 1)
            {
                char c = d & 0xFF;
                if (c == 0)
                    return *this;
                std::string::push_back(c);
                d >>= 8;
            }
            __s += 4;
        }
        return *this;
    }
    using std::string::operator+=;
    string& operator+=(const char* __s) { return append(__s); }
};
inline string operator+ (const string& lhs, const char* rhs) { return string(lhs) += rhs; }
inline string operator+ (const char* lhs, const string& rhs) { return string(lhs) += rhs; }
#endif
#endif
