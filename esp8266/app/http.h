#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void http_init(int port);
void http_regist(const char* url, const char* type, bool (*handler)(void* arg, const char* url, int line));
void http_redirect(void* arg, const char* url);
bool http_chunk_send(void* arg, int line, const char* data, size_t data_length);
void http_parameter_parse(const char* url, void (*parser)(void* context, const char* key, const char* value), void* context);

#ifdef __cplusplus
}
#endif
