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
#define LFS_NO_DEBUG
#define LFS_NO_WARN
#define LFS_NO_ERROR
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
#undef TCP_MSS
#undef _TIME_T_
#define EBUF_LWIP
#define PBUF_RSV_FOR_WLAN
#define TCP_MSS 536
#define _TIME_T_ long
#include <sys/timeb.h>
#include <sys/_tz_structs.h>
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
#include <sys/pgmspace.h>
#undef ICACHE_FLASH_ATTR
#undef ICACHE_RODATA_ATTR
#undef PROGMEM
#define ICACHE_FLASH_ATTR   __attribute__((section(".irom0.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#define ICACHE_RODATA_ATTR  __attribute__((section(".irom.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#define PROGMEM             __attribute__((section(".irom.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))
#define IRAM_ATTR           __attribute__((section(".iram.text." __FILE_NAME__ "." __STRINGIZE(__LINE__))))

#endif
