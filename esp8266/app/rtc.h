#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void rtc_reset();
void rtc_begin();
void rtc_read(int offset, void* value, int size);
void rtc_write(int offset, void* value, int size);

#ifdef __cplusplus
}
#endif
