#include "esp8266.h"
#include <spi_flash.h>
#define USE_SPIFFS 0
#if USE_SPIFFS
#include "spiffs/src/spiffs.h"
#include "spiffs/src/spiffs_nucleus.h"
#else
#include "littlefs/lfs.h"
#endif

#define IROM_ADDR   0x40200000
#define PHYS_BEGIN  0x40400000
#define PHYS_END    0x405FA000
#define PHYS_ADDR   (PHYS_BEGIN - IROM_ADDR)
#define PHYS_SIZE   (PHYS_END - PHYS_BEGIN)
#define PHYS_PAGE   0x100
#define PHYS_BLOCK  0x2000

int32_t fs_hal_read(uint32_t addr, uint32_t size, uint8_t *dst)
{
    uint32_t buffer[PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < PHYS_PAGE ? size : PHYS_PAGE;
        int length_aligned = (length + 3) & ~3;
        SpiFlashOpResult result = spi_flash_read(addr, buffer, length_aligned);
        if (result != SPI_FLASH_RESULT_OK)
        {
            os_printf("%d = %s(%p, %p, %d)\n", result, "spi_flash_read", addr, buffer, length_aligned);
            return -1;
        }
        memcpy(dst, buffer, length);
        addr += length;
        size -= length;
    }
    return 0;
}

int32_t fs_hal_write(uint32_t addr, uint32_t size, uint8_t *src)
{
    uint32_t buffer[PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < PHYS_PAGE ? size : PHYS_PAGE;
        int length_aligned = (length + 3) & ~3;
        memcpy(buffer, src, length);
        SpiFlashOpResult result = spi_flash_write(addr, buffer, length_aligned);
        if (result != SPI_FLASH_RESULT_OK)
        {
            os_printf("%d = %s(%p, %p, %d)\n", result, "spi_flash_write", addr, buffer, length_aligned);
            return -1;
        }
        addr += length;
        size -= length;
    }
    return 0;
}

int32_t fs_hal_erase(uint32_t addr, uint32_t size)
{
    const uint32_t sector = addr / SPI_FLASH_SEC_SIZE;
    const uint32_t sectorCount = size / SPI_FLASH_SEC_SIZE;
    for (uint32_t i = 0; i < sectorCount; ++i)
    {
        SpiFlashOpResult result = spi_flash_erase_sector(sector + i);
        if (result != SPI_FLASH_RESULT_OK)
        {
            os_printf("%d = %s(%p)\n", result, "spi_flash_erase_sector", sector + i);
            return -1;
        }
    }
    return 0;
}

#if USE_SPIFFS
static spiffs fs;
#else
static lfs_t fs;

int lfs_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK) + off;
    return fs_hal_read(addr, size, (uint8_t*)dst);
}

int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK) + off;
    return fs_hal_write(addr, size, (uint8_t*)buffer);
}

int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK);
    uint32_t size = PHYS_BLOCK;
    return fs_hal_erase(addr, size);
}

int lfs_flash_sync(const struct lfs_config *c)
{
    return 0;
}
#endif

void fs_init()
{
#if USE_SPIFFS
    if (SPIFFS_mounted(&fs))
        return;

    static u8_t spiffs_work_buf[PHYS_PAGE * 2];
    static u8_t spiffs_fds[sizeof(spiffs_fd) * 4];
    static u8_t spiffs_cache_buf[sizeof(spiffs_cache) + 4 * (sizeof(spiffs_cache_page) + PHYS_PAGE)];

    static spiffs_config cfg = {};
    cfg.phys_size = PHYS_SIZE;
    cfg.phys_addr = PHYS_ADDR;
    cfg.phys_erase_block = SPI_FLASH_SEC_SIZE;
    cfg.log_block_size = PHYS_BLOCK;
    cfg.log_page_size = PHYS_PAGE;

    cfg.hal_read_f = fs_hal_read;
    cfg.hal_write_f = fs_hal_write;
    cfg.hal_erase_f = fs_hal_erase;

    for (int i = 0; i < 2; ++i)
    {
        if (SPIFFS_mount(&fs,
                         &cfg,
                         spiffs_work_buf,
                         spiffs_fds,
                         sizeof(spiffs_fds),
                         spiffs_cache_buf,
                         sizeof(spiffs_cache_buf),
                         0) == SPIFFS_OK)
            break;
        SPIFFS_format(&fs);
    }
#else
    static struct lfs_config cfg = {};
    cfg.read = lfs_flash_read;
    cfg.prog = lfs_flash_prog;
    cfg.erase = lfs_flash_erase;
    cfg.sync = lfs_flash_sync;
    cfg.read_size = 64;
    cfg.prog_size = 64;
    cfg.block_size = PHYS_BLOCK;
    cfg.block_count = PHYS_SIZE / PHYS_BLOCK;
    cfg.block_cycles = 16;
    cfg.cache_size = 64;
    cfg.lookahead_size = 64;

    if (lfs_mount(&fs, &cfg) != LFS_ERR_OK)
    {
        lfs_format(&fs, &cfg);
        lfs_mount(&fs, &cfg);
    }
#endif
}

int fs_open(const char* name, const char* mode)
{
    char* temp = strdup(name);
#if USE_SPIFFS
    int fd = SPIFFS_open(&fs, temp, pgm_read_byte(mode) == 'w' ? SPIFFS_O_CREAT | SPIFFS_O_WRONLY : SPIFFS_O_RDONLY, 0);
#else
    lfs_file_t* fd = os_zalloc(sizeof(lfs_file_t));
    int result = lfs_file_open(&fs, fd, temp, pgm_read_byte(mode) == 'w' ? LFS_O_CREAT | LFS_O_WRONLY : LFS_O_RDONLY);
    if (result != LFS_ERR_OK)
    {
        os_printf("%d = %s(%p, %p, %s, %s)\n", result, "lfs_file_open", &fs, fd, name, mode);

        os_free(fd);
        fd = (lfs_file_t*)result;
    }
#endif
    os_free(temp);
    return (int)fd;
}

void fs_close(int fd)
{
#if USE_SPIFFS
    SPIFFS_close(&fs, fd);
#else
    lfs_file_close(&fs, (lfs_file_t*)fd);
    os_free((lfs_file_t*)fd);
#endif
}

int fs_stat(int fd)
{
#if USE_SPIFFS
    spiffs_stat stat = {};
    SPIFFS_fstat(&fs, fd, &stat);
    return stat.size;
#else
    return 0;
#endif
}

char* fs_gets(char* buffer, int length, int fd)
{
#if USE_SPIFFS
    int pos = SPIFFS_tell(&fs, fd);
    length = SPIFFS_read(&fs, fd, buffer, length - 1);
#else
    int pos = lfs_file_tell(&fs, (lfs_file_t*)fd);
    length = lfs_file_read(&fs, (lfs_file_t*)fd, buffer, length - 1);
#endif
    buffer[length] = 0;

    for (int i = 0; i < length; ++i)
    {
        char c = buffer[i];
        if (c == 0 || c == '\n')
        {
#if USE_SPIFFS
            SPIFFS_lseek(&fs, fd, pos + i + 1, SPIFFS_SEEK_SET);
#else
            lfs_file_seek(&fs, (lfs_file_t*)fd, pos + i + 1, LFS_SEEK_SET);
#endif
            buffer[i] = 0;
            break;
        }
    }

    return buffer;
}

int fs_read(void* buffer, int length, int fd)
{
#if USE_SPIFFS
    return SPIFFS_read(&fs, fd, buffer, length);
#else
    return lfs_file_read(&fs, (lfs_file_t*)fd, buffer, length);
#endif
}

int fs_write(const void* buffer, int length, int fd)
{
#if USE_SPIFFS
    return SPIFFS_write(&fs, fd, (void*)buffer, length);
#else
    return lfs_file_write(&fs, (lfs_file_t*)fd, (void*)buffer, length);
#endif
}
