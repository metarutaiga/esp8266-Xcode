#include "eagle.h"

static void system_rtc_mem_read(size_t addr, const void* data, size_t size)
{
    addr = 0x60001100 + addr * 4;
    uint32_t* values = (uint32_t*)data;
    for (size_t i = 0; i < size; i += 4)
    {
        *values++ = REG_READ(addr + i);
    }
}

static void system_rtc_mem_write(size_t addr, const void* data, size_t size)
{
    addr = 0x60001100 + addr * 4;
    const uint32_t* values = (uint32_t*)data;
    for (size_t i = 0; i < size; i += 4)
    {
        REG_WRITE(addr + i, *values++);
    }
}

static void rtc_update(uint32_t data[128])
{
    uint32_t first_crc = lfs_crc(0xFFFFFFFF, data + 0, 31 * sizeof(uint32_t));
    uint32_t second_crc = lfs_crc(0xFFFFFFFF, data + 32, 95 * sizeof(uint32_t));
    memcpy(data + 31, &first_crc, sizeof(uint32_t));
    memcpy(data + 127, &second_crc, sizeof(uint32_t));
    system_rtc_mem_write(64, data, 128 * sizeof(uint32_t));
}

void rtc_reset()
{
    uint32_t data[128];
    memset(data, 0xFF, 128 * sizeof(uint32_t));
    rtc_update(data);
}

void rtc_begin()
{
    bool update = false;
    uint32_t data[128];
    system_rtc_mem_read(64, data, 128 * sizeof(uint32_t));
    uint32_t first_crc = lfs_crc(0xFFFFFFFF, data + 0, 31 * sizeof(uint32_t));
    if (memcmp(data + 31, &first_crc, sizeof(uint32_t)) != 0)
    {
        memset(data + 0, 0xFF, 31 * sizeof(uint32_t));
        update = true;
    }
    uint32_t second_crc = lfs_crc(0xFFFFFFFF, data + 32, 95 * sizeof(uint32_t));
    if (memcmp(data + 127, &second_crc, sizeof(uint32_t)) != 0)
    {
        memset(data + 32, 0xFF, 95 * sizeof(uint32_t));
        update = true;
    }
    if (update)
    {
        rtc_update(data);
    }
}

void rtc_read(int offset, void* value, int size)
{
    uint32_t data[128];
    system_rtc_mem_read(64 + offset / 4, data, ((offset % 4 + size + 3) & ~3));
    memcpy(value, (char*)data + offset % 4, size);
}

void rtc_write(int offset, void* value, int size)
{
    uint32_t data[128];
    system_rtc_mem_read(64, data, 128 * sizeof(uint32_t));
    memcpy((char*)data + offset, value, size);
    rtc_update(data);
}
