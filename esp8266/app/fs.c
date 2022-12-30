#include "esp8266.h"
#include <spi_flash.h>
#include "littlefs/lfs.h"

#define IROM_ADDR   0x40200000
#define PHYS_BEGIN  0x40400000
#define PHYS_END    0x405FA000
#define PHYS_ADDR   (PHYS_BEGIN - IROM_ADDR)
#define PHYS_SIZE   (PHYS_END - PHYS_BEGIN)
#define PHYS_PAGE   0x100
#define PHYS_BLOCK  0x2000

static int32_t fs_hal_read(uint32_t addr, uint32_t size, uint8_t* dst)
{
    uint32_t buffer[PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < PHYS_PAGE ? size : PHYS_PAGE;
        int length_aligned = (length + 3) & ~3;
        SpiFlashOpResult result = spi_flash_read(addr, buffer, length_aligned);
        if (result != SPI_FLASH_RESULT_OK)
        {
            os_printf("%d = %s(%p, %p, %d)\n", result, "spi_flash_read", (char*)addr, (char*)buffer, length_aligned);
            return -1;
        }
        os_memcpy(dst, buffer, length);
        addr += length;
        size -= length;
    }
    return 0;
}

static int32_t fs_hal_write(uint32_t addr, uint32_t size, uint8_t* src)
{
    uint32_t buffer[PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < PHYS_PAGE ? size : PHYS_PAGE;
        int length_aligned = (length + 3) & ~3;
        os_memcpy(buffer, src, length);
        SpiFlashOpResult result = spi_flash_write(addr, buffer, length_aligned);
        if (result != SPI_FLASH_RESULT_OK)
        {
            os_printf("%d = %s(%p, %p, %d)\n", result, "spi_flash_write", (char*)addr, (char*)buffer, length_aligned);
            return -1;
        }
        addr += length;
        size -= length;
    }
    return 0;
}

static int32_t fs_hal_erase(uint32_t addr, uint32_t size)
{
    const uint32_t sector = addr / SPI_FLASH_SEC_SIZE;
    const uint32_t sectorCount = size / SPI_FLASH_SEC_SIZE;
    for (uint32_t i = 0; i < sectorCount; ++i)
    {
        SpiFlashOpResult result = spi_flash_erase_sector(sector + i);
        if (result != SPI_FLASH_RESULT_OK)
        {
            os_printf("%d = %s(%p)\n", result, "spi_flash_erase_sector", (char*)sector + i);
            return -1;
        }
    }
    return 0;
}

static lfs_t fs;

static int lfs_flash_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* dst, lfs_size_t size)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK) + off;
    return fs_hal_read(addr, size, (uint8_t*)dst);
}

static int lfs_flash_prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK) + off;
    return fs_hal_write(addr, size, (uint8_t*)buffer);
}

static int lfs_flash_erase(const struct lfs_config* c, lfs_block_t block)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK);
    uint32_t size = PHYS_BLOCK;
    return fs_hal_erase(addr, size);
}

static int lfs_flash_sync(const struct lfs_config* c)
{
    return 0;
}

void fs_init()
{
    static struct lfs_config cfg IRAM_ATTR = {};
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
}

int fs_stat(const char* name)
{
    struct lfs_info info = {};
    lfs_stat(&fs, name, &info);
    return info.size;
}

int fs_open(const char* name, const char* mode)
{
    char* temp = strdup(name);
    lfs_file_t* fd = os_zalloc(sizeof(lfs_file_t));
    int result = lfs_file_open(&fs, fd, temp, pgm_read_byte(mode) == 'w' ? LFS_O_CREAT | LFS_O_WRONLY : LFS_O_RDONLY);
    if (result != LFS_ERR_OK)
    {
        os_printf("%d = %s(%p, %p, %s, %s)\n", result, "lfs_file_open", &fs, fd, name, mode);

        os_free(fd);
        fd = (lfs_file_t*)result;
    }
    os_free(temp);
    return (int)fd;
}

void fs_close(int fd)
{
    if (fd < 0)
        return;
    lfs_file_close(&fs, (lfs_file_t*)fd);
    os_free((lfs_file_t*)fd);
}

int fs_getc(int fd)
{
    if (fd < 0)
        return -1;
    uint8_t c;
    if (lfs_file_read(&fs, (lfs_file_t*)fd, &c, 1) != 1)
        return -1;
    return c;
}

char* fs_gets(char* buffer, int length, int fd)
{
    if (fd < 0)
        return "";

    int pos = lfs_file_tell(&fs, (lfs_file_t*)fd);
    length = lfs_file_read(&fs, (lfs_file_t*)fd, buffer, length - 1);
    buffer[length] = 0;

    for (int i = 0; i < length; ++i)
    {
        char c = buffer[i];
        if (c == 0 || c == '\r' || c == '\n')
        {
            buffer[i] = '\0';
            if (c && (i + 1) < length)
            {
                c = buffer[i + 1];
                if (c == '\r' || c == '\n')
                {
                    ++i;
                }
            }
            lfs_file_seek(&fs, (lfs_file_t*)fd, pos + i + 1, LFS_SEEK_SET);
            break;
        }
    }

    return buffer;
}

void fs_seek(int pos, int fd)
{
    if (fd < 0)
        return;
    lfs_file_seek(&fs, (lfs_file_t*)fd, pos, LFS_SEEK_SET);
}

int fs_tell(int fd)
{
    if (fd < 0)
        return 0;
    return lfs_file_tell(&fs, (lfs_file_t*)fd);
}

int fs_read(void* buffer, int length, int fd)
{
    if (fd < 0)
        return 0;
    return lfs_file_read(&fs, (lfs_file_t*)fd, buffer, length);
}

int fs_write(const void* buffer, int length, int fd)
{
    if (fd < 0)
        return 0;
    return lfs_file_write(&fs, (lfs_file_t*)fd, (void*)buffer, length);
}
