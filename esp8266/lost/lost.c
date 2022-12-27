#include "esp8266.h"
#include <stdlib.h>
#include <time.h>

char* itoa(int value, char* str, int base)
{
    if (base == 16)
        os_sprintf(str, "%x", value);
    else
        os_sprintf(str, "%d", value);
    return str;
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
    long result = 0;
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
