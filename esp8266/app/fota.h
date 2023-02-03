#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void fota(const char* url);
void fota_callback(void* arg);

#ifdef __cplusplus
}
#endif
