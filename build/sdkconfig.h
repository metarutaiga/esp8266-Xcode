#pragma once

#pragma clang diagnostic ignored "-Wmacro-redefined"

#define LFS_NAME_MAX 32
#define LFS_NO_ASSERT
#define LFS_NO_DEBUG
#define LFS_NO_ERROR
#define LFS_NO_WARN

#define BSS_IRAM_ATTR _SECTION_ATTR_IMPL(".bss.iram1", __COUNTER__)
