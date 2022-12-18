#include "esp8266.h"
#include <string>

extern "C" int conv_str_decimal(const char* str);
extern "C" int conv_str_hex(const char* str);

extern "C" char* itoa(int value, char* str, int base)
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

extern "C" char* strdup(const char* str)
{
    int len = os_strlen(str);
    char* dup = (char*)os_malloc(len + 1);
    os_memcpy(dup, str, len);
    dup[len] = 0;
    return dup;
}

extern "C" long strtol(const char* str, char** str_end, int base)
{
    if (base == 16)
        return conv_str_decimal(str);
    return conv_str_hex(str);
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
