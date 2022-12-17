#include "esp8266.h"
#include <string>

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
