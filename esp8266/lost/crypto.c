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

#define AES_PRIV_SIZE (4 * 4 * 15 + 4)
#define AES_PRIV_NR_POS (4 * 15)

void* aes_decrypt_init(const u8* key, size_t len)
{
    extern void rijndaelKeySetupDec(u32 rk[], const u8 cipherKey[]);
    u32* rk;
    if (len != 16)
        return NULL;
    rk = (u32*)os_malloc(AES_PRIV_SIZE);
    if (rk == NULL)
        return NULL;
    rijndaelKeySetupDec(rk, key);
    rk[AES_PRIV_NR_POS] = 10;
    return rk;
}

void aes_decrypt_deinit(void *ctx)
{
    os_memset(ctx, 0, AES_PRIV_SIZE);
    os_free(ctx);
}

void* aes_encrypt_init(const u8* key, size_t len)
{
    extern void rijndaelKeySetupEnc(u32 rk[], const u8 cipherKey[]);
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

void aes_encrypt_deinit(void *ctx)
{
    os_memset(ctx, 0, AES_PRIV_SIZE);
    os_free(ctx);
}

int aes_wrap(const u8 *kek, int n, const u8 *plain, u8 *cipher)
{
    int aes_wrap5(const u8 *kek, size_t kek_len, int n, const u8 *plain, u8 *cipher);
    return aes_wrap5(kek, 16, n, plain, cipher);
}

int aes_unwrap(const u8 *kek, int n, const u8 *cipher, u8 *plain)
{
    int aes_unwrap5(const u8 *kek, size_t kek_len, int n, const u8 *cipher, u8 *plain);
    return aes_unwrap5(kek, 16, n, cipher, plain);
}

void randombytes(u8* data, u64 size)
{
    os_get_random(data, size);
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
