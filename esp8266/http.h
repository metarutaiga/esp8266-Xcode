#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void http_init(int port);
void http_regist(const char *url, const char *type, bool (*handler)(void *arg, int line));
bool http_chunk_send(void *arg, int line, const char *data, size_t data_length);

#ifdef __cplusplus
}
#endif
