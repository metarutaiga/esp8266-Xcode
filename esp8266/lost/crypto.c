#include "esp8266.h"
#include <time.h>

int os_mktime(int year, int month, int day, int hour, int min, int sec, time_t* t)
{
    if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 ||
        sec > 60)
        return -1;

    struct tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;

    *t = (time_t) mktime(&tm);
    return 0;
}

void* aes_encrypt_init(const u8* key, size_t len)
{
#define AES_PRIV_SIZE (4 * 4 * 15 + 4)
#define AES_PRIV_NR_POS (4 * 15)
    u32* rk;
    if (len != 16)
        return NULL;
    rk = (u32*)os_malloc(AES_PRIV_SIZE);
    if (rk == NULL)
        return NULL;
    rijndaelKeySetupEnc(rk, key);
    rk[AES_PRIV_NR_POS] = 10;
    return rk;
}

void wpa_printf(int level, const char* fmt, ...)
{
    extern int ets_putc(int);
    extern int ets_vprintf(int(*putc)(int), const char* fmt, va_list ap);
    va_list args;
    va_start(args, fmt);
    ets_vprintf(ets_putc, fmt, args);
    va_end(args);
    ets_putc('\r');
    ets_putc('\n');
}

void wpa_hexdump(int level, const char* title, const void* buf, size_t len)
{
    
}

void wpa_hexdump_key(int level, const char* title, const void* buf, size_t len)
{
    
}

void wpa_hexdump_ascii(int level, const char* title, const void* buf, size_t len)
{
    
}

void wpa_hexdump_ascii_key(int level, const char* title, const void* buf, size_t len)
{
    
}
