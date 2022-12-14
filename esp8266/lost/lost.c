#include "esp8266.h"
#include <stdlib.h>
#include <time.h>

int atoi(const char* str)
{
    return strtol(str, NULL, 10);
}

char* itoa(int value, char* str, int base)
{
    if (base == 16)
        os_sprintf(str, "%x", value);
    else
        os_sprintf(str, "%d", value);
    return str;
}

void* memchr(const void* src_void, int c, size_t length)
{
    const unsigned char *src = src_void;
    unsigned char d = c;

    while (length--)
    {
        if (pgm_read_byte(src) == d)
            return (void*)src;
        src++;
    }

    return NULL;
}

time_t mktime(struct tm* tim_p)
{
    static const int _DAYS_BEFORE_MONTH[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

#define _SEC_IN_MINUTE 60L
#define _SEC_IN_HOUR 3600L
#define _SEC_IN_DAY 86400L
#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y)+1900) % 400) == 0))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)

    time_t tim = 0;
    long days = 0;
    int year = 0;

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
    tim_p->tm_isdst = 0;

    /* compute day of the week */
    if ((tim_p->tm_wday = (days + 4) % 7) < 0)
        tim_p->tm_wday += 7;

    return tim;
}

int strcasecmp(const char* s1, const char* s2)
{
    return os_strcmp(s1, s2);
}

char* strchr(const char* str, int chr)
{
    unsigned char b;
    unsigned char c = chr;
    while ((b = pgm_read_byte(str)) && b != c)
        str++;
    if (b == c)
        return (char*)str;
    return NULL;
}

size_t strcspn(const char* str, const char* spn)
{
    const char* s = str;
    char b;
    char c;

    while ((b = pgm_read_byte(str)))
    {
        for (const char* p = spn; (c = pgm_read_byte(p)); p++) if (b == c) break;
        if (c) break;
        str++;
    }

    return str - s;
}

char* strdup(const char* str)
{
    int len = os_strlen(str);
    char* dup = (char*)os_malloc(len + 1);
    for (int i = 0; i < len; ++i)
        dup[i] = pgm_read_byte(str + i);
    dup[len] = 0;
    return dup;
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
    char b;
    char c;

    while ((b = pgm_read_byte(str)))
    {
        for (const char* p = spn; (c = pgm_read_byte(p)); p++) if (b == c) break;
        if (c == '\0') break;
        str++;
    }

    return str - s;
}

long strtol(const char* str, char** str_end, int base)
{
    long result = 0;
    if (base == 16)
    {
        for (char c = 0; (c = *str); str++)
        {
            if (c >= 'a' && c <= 'f')
                result = result * 16 + c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                result = result * 16 + c - 'A' + 10;
            else if (c >= '0' && c <= '9')
                result = result * 16 + c - '0';
            else
                break;
        }
    }
    else
    {
        for (char c = 0; (c = *str); str++)
        {
            if (c >= '0' && c <= '9')
                result = result * 10 + c - '0';
            else
                break;
        }
    }
    return result;
}
