#pragma once

#pragma clang diagnostic ignored "-Wmacro-redefined"

#define __ESP_FILE__ __FILE_NAME__

#define LFS_NAME_MAX 32
#define LFS_NO_ASSERT
#define LFS_NO_DEBUG
#define LFS_NO_ERROR
#define LFS_NO_WARN

#include <esp_attr.h>

#define IRAM_BSS_ATTR _SECTION_ATTR_IMPL(".bss.iram1", __COUNTER__)
