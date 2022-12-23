#include "esp8266.h"
#include <time.h>
#include <string>

char* itoa(int value, char* str, int base)
{
    if (base == 16)
        os_sprintf(str, "%x", value);
    else
        os_sprintf(str, "%d", value);
    return str;
}

time_t mktime(struct tm *tim_p)
{
    static const int _DAYS_BEFORE_MONTH[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

#define _SEC_IN_MINUTE 60L
#define _SEC_IN_HOUR 3600L
#define _SEC_IN_DAY 86400L
#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y)+1900) % 400) == 0))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)

    time_t tim = 0;
    long days = 0;
    int year, isdst=0;

    /* compute hours, minutes, seconds */
    tim += tim_p->tm_sec + (tim_p->tm_min * _SEC_IN_MINUTE) + (tim_p->tm_hour * _SEC_IN_HOUR);

    /* compute days in year */
    days += tim_p->tm_mday - 1;
    days += _DAYS_BEFORE_MONTH[tim_p->tm_mon];
    if (tim_p->tm_mon > 1 && _DAYS_IN_YEAR (tim_p->tm_year) == 366)
        days++;

    /* compute day of the year */
    tim_p->tm_yday = days;

    if (tim_p->tm_year > 10000 || tim_p->tm_year < -10000)
        return (time_t) -1;

    /* compute days in other years */
    if ((year = tim_p->tm_year) > 70)
    {
        for (year = 70; year < tim_p->tm_year; year++)
            days += _DAYS_IN_YEAR (year);
    }
    else if (year < 70)
    {
        for (year = 69; year > tim_p->tm_year; year--)
            days -= _DAYS_IN_YEAR (year);
        days -= _DAYS_IN_YEAR (year);
    }

    /* compute total seconds */
    tim += (days * _SEC_IN_DAY);

    /* reset isdst flag to what we have calculated */
    tim_p->tm_isdst = isdst;

    /* compute day of the week */
    if ((tim_p->tm_wday = (days + 4) % 7) < 0)
        tim_p->tm_wday += 7;

    return tim;
}

extern "C" int os_mktime(int year, int month, int day, int hour, int min, int sec, time_t* t)
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

extern "C" int snprintf(char* str, unsigned int size, const char* format, ...)
{
    extern int ets_vsnprintf(char* str, unsigned int size, const char* format, va_list arg);

    va_list args;
    va_start(args, format);
    int result = ets_vsnprintf(str, size, format, args);
    va_end(args);
    return result;
}

extern "C" int strcasecmp(const char* s1, const char* s2)
{
    return os_strcmp(s1, s2);
}

char* strchr(const char* str, int chr)
{
    const unsigned char* s = (const unsigned char*)str;
    unsigned char c = chr;
    while (*s && *s != c)
        s++;
    if (*s == c)
        return (char*)s;
    return NULL;
}

size_t strcspn(const char* str, const char* spn)
{
    const char* s = str;
    char c;

    while (*str)
    {
        for (const char* p = spn; (c = pgm_read_byte(p)); p++) if (*str == c) break;
        if (c) break;
        str++;
    }

    return str - s;
}

char* strdup(const char* str)
{
    int len = os_strlen(str);
    char* dup = (char*)os_malloc(len + 1);
    os_memcpy(dup, str, len);
    dup[len] = 0;
    return dup;
}

extern "C" size_t os_strlcpy(char* dest, const char* src, size_t siz)
{
    const char* s = src;
    size_t left = siz;

    if (left) {
        /* Copy string up to the maximum size of the dest buffer */
        while (--left != 0) {
            if ((*dest++ = pgm_read_byte(s++)) == '\0')
                break;
        }
    }

    if (left == 0) {
        /* Not enough room for the string; force NUL-termination */
        if (siz != 0)
            *dest = '\0';
        while (pgm_read_byte(s++))
            ; /* determine total src string length */
    }

    return s - src - 1;
}

char* strsep(char** sp, const char* sep)
{
    if (sp == 0 || *sp == 0 || **sp == '\0')
        return NULL;
    char* s = *sp;
    char* p = s + strcspn(s, sep);
    if (*p != '\0')
        *p++ = '\0';
    *sp = p;
    return s;
}

size_t strspn(const char* str, const char* spn)
{
    const char* s = str;
    char c;

    while (*str)
    {
        for (const char* p = spn; (c = pgm_read_byte(p)); p++) if (*str == c) break;
        if (c == '\0') break;
        str++;
    }

    return str - s;
}

long strtol(const char* str, char** str_end, int base)
{
    int result = 0;
    if (base == 16)
    {
        for (char c = 0; (c = *str); str++)
        {
            if (c >= 'a')
                result = result * 16 + c - 'a' + 10;
            else if (c >= 'A')
                result = result * 16 + c - 'A' + 10;
            else
                result = result * 16 + c - '0';
        }
    }
    else
    {
        for (char c = 0; (c = *str); str++)
        {
            result = result * 10 + c - '0';
        }
    }
    return result;
}

extern "C" void* __hide_aliasing_typecast(void* foo)
{
    return foo;
}

extern "C" void forced_memzero(void* ptr, size_t len)
{
    memset(ptr, 0, len);
}

extern "C" int os_memcmp_const(const void* a, const void* b, size_t len)
{
    return os_memcmp(a, b, len);
}

extern "C" void* os_memdup(const void* src, size_t len)
{
    char* dup = (char*)os_malloc(len);
    os_memcpy(dup, src, len);
    return dup;
}

extern "C" void* aes_encrypt_init(const u8* key, size_t len)
{
#define AES_PRIV_SIZE (4 * 4 * 15 + 4)
#define AES_PRIV_NR_POS (4 * 15)
    u32* rk;
    if (len != 16)
        return NULL;
    rk = (u32 *)os_malloc(AES_PRIV_SIZE);
    if (rk == NULL)
        return NULL;
    extern void rijndaelKeySetupEnc(u32 rk[], const u8 cipherKey[]);
    rijndaelKeySetupEnc(rk, key);
    rk[AES_PRIV_NR_POS] = 10;
    return rk;
}

void* operator new(size_t size)
{
    return os_malloc(size);
}

void* operator new[](size_t size)
{
    return os_malloc(size);
}

void operator delete(void* ptr) noexcept
{
    os_free(ptr);
}

void operator delete(void* ptr, size_t size) noexcept
{
    os_free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    os_free(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept
{
    os_free(ptr);
}

#pragma clang diagnostic ignored "-Winvalid-noreturn"
void abort()
{
    
}

#if defined(_LIBCPP_VERSION)
_LIBCPP_BEGIN_NAMESPACE_STD
    template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_string<char>;
_LIBCPP_END_NAMESPACE_STD
#endif
