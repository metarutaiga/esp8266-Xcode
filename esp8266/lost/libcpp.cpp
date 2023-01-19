#include <stdio.h>
#include <new>
#include <string>

#if defined(_LIBCPP_VERSION)
_LIBCPP_BEGIN_NAMESPACE_STD
    template class _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS basic_string<char>;
_LIBCPP_END_NAMESPACE_STD
#endif

namespace std
{
    const nothrow_t nothrow {};
}

void* operator new(size_t size)
{
    return malloc(size);
}

void* operator new(size_t size, std::nothrow_t const&) noexcept
{
    return malloc(size);
}

void* operator new[](size_t size)
{
    return malloc(size);
}

void* operator new[](size_t size, std::nothrow_t const&) noexcept
{
    return malloc(size);
}

void operator delete(void* ptr) noexcept
{
    free(ptr);
}

void operator delete(void* ptr, size_t size) noexcept
{
    free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    free(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept
{
    free(ptr);
}
