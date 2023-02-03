#pragma once

#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wsection"

#define __ESP_FILE__ __FILE_NAME__

#define LFS_NAME_MAX 32
#define LFS_NO_ASSERT
#define LFS_NO_DEBUG
#define LFS_NO_ERROR
#define LFS_NO_WARN

#include <esp_attr.h>

#undef _SECTION_ATTR_IMPL
#if defined(__XTENSA__)
#define _SECTION_ATTR_IMPL(SECTION, COUNTER) __attribute__((section(SECTION "." _COUNTER_STRINGIFY(COUNTER))))
#else
#define _SECTION_ATTR_IMPL(SECTION, COUNTER)
#endif

#define IRAM_BSS_ATTR _SECTION_ATTR_IMPL(".bss.iram1", __COUNTER__)
#define RODATA_STR_ATTR _SECTION_ATTR_IMPL(".rodata.str1.1", __COUNTER__)
