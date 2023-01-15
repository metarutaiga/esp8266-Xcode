/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON_config_h
#define cJSON_config_h

#include <ctype.h>
#include <math.h>
#include <osapi.h>
#include <stdio.h>

#include <nanoprintf/nanoprintf.h>

#undef isinf
#undef isnan
#undef sprintf
#undef sscanf
#undef tolower
#define memcpy os_memcpy
#define memset os_memset
#define sprintf(a, b, ...) npf_snprintf(a, sizeof(a), strncmp(b, "%1.1", 4) == 0 ? "%f" : b, ## __VA_ARGS__)
#define sscanf(...) 0
#define strcmp os_strcmp
#define strcpy os_strcpy
#define strlen os_strlen
#define strncmp os_strncmp
#define tolower(c) ((c >= 'A' && c <= 'Z') ? (c + 0x20) : c)

#endif
