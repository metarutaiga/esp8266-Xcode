#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void https_connect(const char* url, const char* attr, void (*recv)(void* arg, char* pusrdata, int length), void (*disconn)(void* arg));
void https_disconnect(void* arg);

void https_send(void* arg, const void* data, int length);

#ifdef __cplusplus
}
#endif
