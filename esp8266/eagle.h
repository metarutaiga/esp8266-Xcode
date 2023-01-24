#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_attr.h>
#include <esp_clk.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

extern char* url_decode(char* param);
extern uint32_t lfs_crc(uint32_t crc, const void* buffer, size_t size);
inline uint32_t IRAM_ATTR esp_get_cycle_count()
{
    uint32_t ccount;
    __asm__ __volatile__("rsr %0,ccount":"=a"(ccount));
    return ccount;
}

extern const char* const version;
extern const char* const build_date;
extern const char* const web_css;
extern char thisname[16];
extern char number[128];

#ifdef __cplusplus
};
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Flash memory must be read using 32 bit aligned addresses else a processor
// exception will be triggered.
// The order within the 32 bit values are:
// --------------
// b3, b2, b1, b0
//     w1,     w0

// Cast `addr` to `const uint32_t*`, by discarding 2 LSBs (byte offset)
#ifdef __cplusplus
    #define __pgm_cast_u32ptr(addr)         reinterpret_cast<const uint32_t*>(reinterpret_cast<uintptr_t>(addr) & ~3)
#else
    #define __pgm_cast_u32ptr(addr)         (const uint32_t*)((uintptr_t)(addr) & ~3)
#endif

// Inline assembler: Adjust `word` to byte offset of `addr`
#define __pgm_adjust_offset(word, addr, res) \
    __asm__ ( \
        "ssa8l\t%2\n\t"  /* SAR = (AR[`addr`] & 3) * 8; */ \
        "srl\t%0, %1"    /* AR[`res`] = AR[`word`] >> SAR; */ \
        : "=r"(res) : "r"(word), "r"(addr))

#define pgm_read_with_offset(addr, res) \
    __pgm_adjust_offset(*__pgm_cast_u32ptr(addr), addr, res)

#define pgm_read_byte(addr)                 (__extension__({uint32_t res; pgm_read_with_offset(addr, res); (uint8_t)res;}))
#ifdef __cplusplus
    #define pgm_read_dword_aligned(addr)    (*reinterpret_cast<const uint32_t*>(addr))
    #define pgm_read_float_aligned(addr)    (*reinterpret_cast<const float*>(addr))
    #define pgm_read_double_aligned(addr)   (*reinterpret_cast<const double*>(addr))
    #define pgm_read_ptr_aligned(addr)      (*reinterpret_cast<const void* const*>(addr))
#else
    #define pgm_read_dword_aligned(addr)    (*(const uint32_t*)(addr))
    #define pgm_read_float_aligned(addr)    (*(const float*)(addr))
    #define pgm_read_double_aligned(addr)   (*(const double*)(addr))
    #define pgm_read_ptr_aligned(addr)      (*(const void* const*)(addr))
#endif

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
