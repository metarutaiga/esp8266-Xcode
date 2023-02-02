#include "eagle.h"

size_t strcspn(const char* str, const char* spn)
{
    const char* s = str;
    char b;
    char c;

    while ((b = pgm_read_byte(str))) {
        for (const char* p = spn; (c = pgm_read_byte(p)); p++)
            if (b == c)
                break;
        if (c)
            break;
        str++;
    }

    return str - s;
}

size_t strspn(const char* str, const char* spn)
{
    const char* s = str;
    char b;
    char c;

    while ((b = pgm_read_byte(str))) {
        for (const char* p = spn; (c = pgm_read_byte(p)); p++)
            if (b == c)
                break;
        if (c == '\0')
            break;
        str++;
    }

    return str - s;
}

char* url_decode(char* param)
{
    int l = 0;
    int r = 0;
    for (;;) {
        char c = param[r++];
        if (c == '%') {
            char temp[3] = { param[r], param[r + 1] };
            c = strtol(temp, 0, 16);
            r += 2;
        }
        param[l++] = c;
        if (c == 0)
            break;
    }
    return param;
}
