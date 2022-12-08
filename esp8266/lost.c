#include "esp8266.h"

char* itoa(int value, char* str, int base)
{
    if (base == 16)
    {
        os_sprintf(str, "%x", value);
    }
    else
    {
        os_sprintf(str, "%d", value);
    }
    return str;
}
