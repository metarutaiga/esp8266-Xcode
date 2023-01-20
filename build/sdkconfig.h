#pragma once

#pragma clang diagnostic ignored "-Wmacro-redefined"

#define pgm_adjust_offset(addr, res)    __asm__("ssa8l\t%1\nsrl\t%0, %1" : "=r"(res) : "r"(addr))
#define pgm_read_byte(addr)             (__extension__({uint32_t res; pgm_adjust_offset((uint32_t)addr, res); res;}))
#define pgm_read_dword_aligned(addr)    (*(const uint32_t*)(addr))
