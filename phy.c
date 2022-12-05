#include <string.h>

const char phy_data[128] PROGMEM =
{
     5,   8,   4,   2,   5,   5,   5,   2,   5,   0,   4,   5,   5,   4,   5,   5,
     4,  -2,  -3,  -1, -16, -16, -16, -32, -32, -32, -31,  10,  -1,  -1,  -8,   0,
    -8,  -8,  78,  74,  70,  64,  60,  56,   0,   0,   1,   1,   2,   3,   4,   5,
     1,   0,   0,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    -31, 10,   0,   0,   0,   0,   0,   0,   0,   0,   1,-109,  67,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

extern int __real_spi_flash_read(uint32_t addr, uint32_t* dst, size_t size);
extern int __wrap_spi_flash_read(uint32_t addr, uint32_t* dst, size_t size);
extern int __wrap_spi_flash_read(uint32_t addr, uint32_t* dst, size_t size)
{
    if (addr == 0x3fc000 && size == 128) {
        memcpy_P(dst, phy_data, sizeof(phy_data));
        return 0;
    }
    return __real_spi_flash_read(addr, dst, size);
}
