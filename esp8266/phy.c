#include "esp8266.h"

const char phy_data[128] =
{
      5,   8,   4,   2,   5,   5,   5,   2,   5,   0,   4,   5,   5,   4,   5,   5,
      4,  -2,  -3,  -1, -16, -16, -16, -32, -32, -32, -31,  10,  -1,  -1,  -8,   0,
     -8,  -8,  78,  74,  70,  64,  60,  56,   0,   0,   1,   1,   2,   3,   4,   5,
      1,   0,   0,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    -31,  10,   0,   0,   0,   0,   0,   0,   0,   0,   1,-109,  67,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

extern int __real_spi_flash_read(uint32_t addr, uint32_t* dst, size_t size);
extern int __wrap_spi_flash_read(uint32_t addr, uint32_t* dst, size_t size);
extern int __wrap_spi_flash_read(uint32_t addr, uint32_t* dst, size_t size)
{
//  os_printf("spi_flash_read(%08x, %p, %d)\n", addr, dst, size);
    if (addr == 0x3fc000 && size == 128)
    {
        memcpy(dst, phy_data, 128);
        return 0;
    }
    return __real_spi_flash_read(addr, dst, size);
}

extern int __real_spi_flash_write(uint32_t addr, const uint32_t* dst, size_t size);
extern int __wrap_spi_flash_write(uint32_t addr, const uint32_t* dst, size_t size);
extern int __wrap_spi_flash_write(uint32_t addr, const uint32_t* dst, size_t size)
{
//  os_printf("spi_flash_write(%08x, %p, %d)\n", addr, dst, size);
    return __real_spi_flash_write(addr, dst, size);
}
