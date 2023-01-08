#include "esp8266.h"

static void rtc_update(uint32_t data[128])
{
    uint32_t first_crc = lfs_crc(0xFFFFFFFF, data + 0, 31 * sizeof(uint32_t));
    uint32_t second_crc = lfs_crc(0xFFFFFFFF, data + 32, 95 * sizeof(uint32_t));
    os_memcpy(data + 31, &first_crc, sizeof(uint32_t));
    os_memcpy(data + 127, &second_crc, sizeof(uint32_t));
    system_rtc_mem_write(64, data, 128 * sizeof(uint32_t));
}

void rtc_reset()
{
    uint32_t data[128];
    os_memset(data, 0xFF, 128 * sizeof(uint32_t));
    rtc_update(data);
}

void rtc_begin()
{
    bool update = false;
    uint32_t data[128];
    system_rtc_mem_read(64, data, 128 * sizeof(uint32_t));
    uint32_t first_crc = lfs_crc(0xFFFFFFFF, data + 0, 31 * sizeof(uint32_t));
    if (os_memcmp(data + 31, &first_crc, sizeof(uint32_t)) != 0) {
        os_memset(data + 0, 0xFF, 31 * sizeof(uint32_t));
        update = true;
    }
    uint32_t second_crc = lfs_crc(0xFFFFFFFF, data + 32, 95 * sizeof(uint32_t));
    if (os_memcmp(data + 127, &second_crc, sizeof(uint32_t)) != 0) {
        os_memset(data + 32, 0xFF, 95 * sizeof(uint32_t));
        update = true;
    }
    if (update) {
        rtc_update(data);
    }
}

void rtc_read(int offset, void* value, int size)
{
    uint32_t data[128];
    system_rtc_mem_read(64 + offset / 4, data, ((offset % 4 + size + 3) & ~3));
    os_memcpy(value, (char*)data + offset % 4, size);
}

void rtc_write(int offset, void* value, int size)
{
    uint32_t data[128];
    system_rtc_mem_read(64, data, 128 * sizeof(uint32_t));
    os_memcpy((char*)data + offset, value, size);
    rtc_update(data);
}
