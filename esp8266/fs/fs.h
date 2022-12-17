#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void fs_init();
int fs_open(const char* name, const char* mode);
void fs_close(int fd);

int fs_stat(int fd);
int fs_read(void* buffer, int length, int fd);
int fs_write(const void* buffer, int length, int fd);

#ifdef __cplusplus
}
#endif
