/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#ifdef LFS_UTIL_H
#undef LFS_CONFIG
#undef LFS_UTIL_H
#ifndef __cplusplus
#define __cplusplus
#include <c_types.h>
#undef __cplusplus
#endif
#define LFS_NAME_MAX 32
#define LFS_NO_ASSERT
#define LFS_NO_DEBUG
#define LFS_NO_ERROR
#define LFS_NO_WARN
#define printf os_printf_plus
#include "littlefs/lfs.h"
#endif

#ifdef LWIP_OPEN_SRC
#pragma clang diagnostic ignored "-Wcomma"
#pragma clang diagnostic ignored "-Wpointer-bool-conversion"
#pragma clang diagnostic ignored "-Wsometimes-uninitialized"
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunused-value"
#pragma clang diagnostic ignored "-Wunused-variable"
#undef DNS_MAX_NAME_LENGTH
#undef DNS_TABLE_SIZE
#undef TCP_MSS
#undef _TIME_T_
#define DNS_MAX_NAME_LENGTH 32
#define DNS_TABLE_SIZE 2
#define EBUF_LWIP
#define PBUF_RSV_FOR_WLAN
#define TCP_MSS 536
#define _TIME_T_ long
#include <time.h>
#undef isprint
#undef isdigit
#undef isxdigit
#undef islower
#undef isspace
#define in_range(c, lo, up)  ((u8_t)c >= lo && (u8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#else
#include <user_interface.h>
#endif

#pragma clang diagnostic ignored "-Wsection"
#include <c_types.h>
#undef ICACHE_FLASH_ATTR
#undef ICACHE_RODATA_ATTR
#undef PROGMEM
#undef IRAM_ATTR
#define __STRINGIZE_NX(A) #A
#define __STRINGIZE(A) __STRINGIZE_NX(A)

#if defined(__XTENSA__)
#define ICACHE_FLASH_ATTR   __attribute__((section(".irom0.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#define ICACHE_RODATA_ATTR  __attribute__((section(".irom.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#define PROGMEM             __attribute__((section(".irom.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#define IRAM_FLASH_ATTR     __attribute__((section(".iram0.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#define IRAM_ATTR           __attribute__((section(".iram.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#else
#define ICACHE_FLASH_ATTR
#define ICACHE_RODATA_ATTR
#define PROGMEM
#define IRAM_FLASH_ATTR
#define IRAM_ATTR
#endif

#define pgm_adjust_offset(addr, res) \
    uint32_t word = *(const uint32_t*)((uint32_t)addr & ~3); \
    uint32_t shift = ((uint32_t)addr & 3) * 8; \
    res = word >> shift;
#define pgm_read_byte(addr)                 (__extension__({uint32_t res; pgm_adjust_offset(addr, res); (uint8_t)res;}))
#define pgm_read_word(addr)                 (__extension__({uint32_t res; pgm_adjust_offset(addr, res); (uint16_t)res;}))
#define pgm_read_dword_aligned(addr)        (*(const uint32_t*)(addr))

#endif
