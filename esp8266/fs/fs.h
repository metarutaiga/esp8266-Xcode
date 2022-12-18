#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void fs_init();
int fs_stat(const char* name);
int fs_open(const char* name, const char* mode);
void fs_close(int fd);

char* fs_gets(char* buffer, int length, int fd);
int fs_read(void* buffer, int length, int fd);
int fs_write(const void* buffer, int length, int fd);

#ifdef __cplusplus
}
#endif
