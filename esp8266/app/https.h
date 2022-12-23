#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void https_connect(const char* url, void (*recv)(char* pusrdata, int length));

#ifdef __cplusplus
}
#endif
