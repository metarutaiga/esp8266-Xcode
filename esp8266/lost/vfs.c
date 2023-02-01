#include "eagle.h"
#include <sys/lock.h>
#include <esp8266/uart_struct.h>
#include <esp_vfs.h>

int __wrap__open_r(struct _reent *r, const char * path, int flags, int mode) { return -1; }
ssize_t __wrap__write_r(struct _reent *r, int fd, const void * data, size_t size) { return -1; }
off_t __wrap__lseek_r(struct _reent *r, int fd, off_t size, int mode) { return 0; }
ssize_t __wrap__read_r(struct _reent *r, int fd, void * dst, size_t size) { return -1; }
int __wrap__close_r(struct _reent *r, int fd) { return -1; }
int __wrap__fstat_r(struct _reent *r, int fd, struct stat * st) { return -1; }
int __wrap__stat_r(struct _reent *r, const char * path, struct stat * st) { return -1; }
int __wrap__link_r(struct _reent *r, const char* n1, const char* n2) { return -1; }
int __wrap__unlink_r(struct _reent *r, const char *path) { return -1; }
int __wrap__rename_r(struct _reent *r, const char *src, const char *dst) { return -1; }

static _lock_t write_lock IRAM_BSS_ATTR;

static int uart_open(const char * path, int flags, int mode)
{
    return 0;
}

static ssize_t uart_write(int fd, const void * data, size_t size)
{
    const char* text = data;
    _lock_acquire_recursive(&write_lock);
    for (size_t i = 0; i < size; ++i) {
        char c = text[i];
        if (c == '\n') {
            while (uart0.status.txfifo_cnt >= 127);
            uart0.fifo.rw_byte = '\r';
        }
        while (uart0.status.txfifo_cnt >= 127);
        uart0.fifo.rw_byte = c;
    }
    _lock_release_recursive(&write_lock);
    return size;
}

static int uart_fstat(int fd, struct stat * st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

void __wrap_esp_vfs_dev_uart_register(void)
{
    esp_vfs_t vfs = {
        .flags = ESP_VFS_FLAG_DEFAULT,
        .write = &uart_write,
        .open = &uart_open,
        .fstat = &uart_fstat,
    };
    ESP_ERROR_CHECK(esp_vfs_register("/dev/uart", &vfs, NULL));
}

esp_err_t __wrap_esp_vfs_register_fd_range(const esp_vfs_t *vfs, void *ctx, int min_fd, int max_fd)
{
    return ESP_FAIL;
}
