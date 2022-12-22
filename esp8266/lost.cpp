#include "esp8266.h"
#include <string>

char* itoa(int value, char* str, int base)
{
    if (base == 16)
        os_sprintf(str, "%x", value);
    else
        os_sprintf(str, "%d", value);
    return str;
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
