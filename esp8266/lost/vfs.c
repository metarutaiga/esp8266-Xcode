#include "eagle.h"
#include <sys/lock.h>
#include <esp8266/uart_struct.h>
#include <esp_vfs.h>
#include <lwip/sockets.h>

void esp_vfs_dev_uart_register(void)
{
}

esp_err_t esp_vfs_register_fd_range(const esp_vfs_t* vfs, void* ctx, int min_fd, int max_fd)
{
    return ESP_FAIL;
}

int esp_vfs_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds, struct timeval* timeout)
{
    return lwip_select(nfds, readfds, writefds, errorfds, timeout);
}

int _open_r(struct _reent* r, const char* path, int flags, int mode)
{
    if (strncmp(path, "/dev/uart/0", 9) == 0)
        return 0;
    return -1;
}

ssize_t _write_r(struct _reent* r, int fd, const void* data, size_t size)
{
    if (fd == 0) {
        static _lock_t write_lock IRAM_BSS_ATTR;
        const char* text = data;
        __lock_acquire_recursive(write_lock);
        for (size_t i = 0; i < size; ++i) {
            char c = text[i];
            if (c == '\n') {
                while (uart0.status.txfifo_cnt >= 127);
                uart0.fifo.rw_byte = '\r';
            }
            while (uart0.status.txfifo_cnt >= 127);
            uart0.fifo.rw_byte = c;
        }
        __lock_release_recursive(write_lock);
        return size;
    }
    return -1;
}

off_t _lseek_r(struct _reent* r, int fd, off_t size, int mode)
{
    return 0;
}

ssize_t _read_r(struct _reent* r, int fd, void* dst, size_t size)
{
    return -1;
}

int _close_r(struct _reent* r, int fd)
{
    return -1;
}

int _fstat_r(struct _reent* r, int fd, struct stat* st)
{
    if (fd == 0) {
        st->st_mode = S_IFCHR;
        return 0;
    }
    return -1;
}

int _stat_r(struct _reent* r, const char*  path, struct stat* st)
{
    return -1;
}

int _link_r(struct _reent* r, const char* n1, const char* n2)
{
    return -1;
}

int _unlink_r(struct _reent* r, const char* path)
{
    return -1;
}

int _rename_r(struct _reent* r, const char* src, const char* dst)
{
    return -1;
}
