#include "esp8266.h"
#include <spi_flash.h>
#include "spiffs/src/spiffs.h"
#include "spiffs/src/spiffs_nucleus.h"

#define SPIFFS_PHYS_BEGIN   0x40400000
#define SPIFFS_PHYS_END     0x405FA000
#define SPIFFS_PHYS_ADDR    (SPIFFS_PHYS_BEGIN - 0x40200000)
#define SPIFFS_PHYS_SIZE    (SPIFFS_PHYS_END - SPIFFS_PHYS_BEGIN)
#define SPIFFS_PHYS_PAGE    0x100
#define SPIFFS_PHYS_BLOCK   0x2000

s32_t fs_hal_read(u32_t addr, u32_t size, u8_t *dst)
{
    uint32_t buffer[SPIFFS_PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < SPIFFS_PHYS_PAGE ? size : SPIFFS_PHYS_PAGE;
        if (spi_flash_read(addr, buffer, length) != SPI_FLASH_RESULT_OK)
        {
            return SPIFFS_ERR_NOT_READABLE;
        }
        memcpy(dst, buffer, length);
        addr += length;
        size -= length;
    }
    return SPIFFS_OK;
}

s32_t fs_hal_write(u32_t addr, u32_t size, u8_t *src)
{
    uint32_t buffer[SPIFFS_PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < SPIFFS_PHYS_PAGE ? size : SPIFFS_PHYS_PAGE;
        memcpy(buffer, src, length);
        if (spi_flash_write(addr, buffer, length) != SPI_FLASH_RESULT_OK)
        {
            return SPIFFS_ERR_NOT_WRITABLE;
        }
        addr += length;
        size -= length;
    }
    return SPIFFS_OK;
}

s32_t fs_hal_erase(u32_t addr, u32_t size)
{
    const uint32_t sector = addr / SPI_FLASH_SEC_SIZE;
    const uint32_t sectorCount = size / SPI_FLASH_SEC_SIZE;
    for (uint32_t i = 0; i < sectorCount; ++i)
    {
        if (spi_flash_erase_sector(sector + i) != SPI_FLASH_RESULT_OK)
        {
            return SPIFFS_ERR_ERASE_FAIL;
        }
    }
    return SPIFFS_OK;
}

static spiffs fs;

void fs_init()
{
    static u8_t spiffs_work_buf[SPIFFS_PHYS_PAGE * 2];
    static u8_t spiffs_fds[sizeof(spiffs_fd) * 4];
    static u8_t spiffs_cache_buf[sizeof(spiffs_cache) + 4 * (sizeof(spiffs_cache_page) + SPIFFS_PHYS_PAGE)];

    spiffs_config cfg = {};
    cfg.phys_size = SPIFFS_PHYS_SIZE;
    cfg.phys_addr = SPIFFS_PHYS_ADDR;
    cfg.phys_erase_block = SPI_FLASH_SEC_SIZE;
    cfg.log_block_size = SPIFFS_PHYS_BLOCK;
    cfg.log_page_size = SPIFFS_PHYS_PAGE;

    cfg.hal_read_f = fs_hal_read;
    cfg.hal_write_f = fs_hal_write;
    cfg.hal_erase_f = fs_hal_erase;

    SPIFFS_mount(&fs,
                 &cfg,
                 spiffs_work_buf,
                 spiffs_fds,
                 sizeof(spiffs_fds),
                 spiffs_cache_buf,
                 sizeof(spiffs_cache_buf),
                 0);
}

int fs_open(const char* name, const char* mode)
{
    char* temp = strdup(name);
    int fd = SPIFFS_open(&fs, temp, pgm_read_byte(mode) == 'w' ? SPIFFS_O_WRONLY : SPIFFS_O_RDONLY, 0);
    os_free(temp);
    os_printf("open : %d = %s %s\n", fd, name, mode);
    return fd;
}

void fs_close(int fd)
{
    SPIFFS_close(&fs, fd);
}

int fs_stat(int fd)
{
    spiffs_stat stat = {};
    SPIFFS_fstat(&fs, fd, &stat);
    return stat.size;
}

char* fs_gets(char* buffer, int length, int fd)
{
    int pos = SPIFFS_tell(&fs, fd);
    length = SPIFFS_read(&fs, fd, buffer, length - 1);
    buffer[length] = 0;

    for (int i = 0; i < length; ++i)
    {
        char c = buffer[i];
        if (c == 0 || c == '\n')
        {
            SPIFFS_lseek(&fs, fd, pos + i + 1, SPIFFS_SEEK_SET);
            buffer[i] = 0;
            break;
        }
    }

    return buffer;
}

int fs_read(void* buffer, int length, int fd)
{
    return SPIFFS_read(&fs, fd, buffer, length);
}

int fs_write(const void* buffer, int length, int fd)
{
    return SPIFFS_write(&fs, fd, (void*)buffer, length);
}
