#include "esp8266.h"
extern "C" {
#include <spi_flash.h>
#include <upgrade.h>
}
#include <string>
#include "https.h"
#include "fota.h"

struct fota_context
{
    int address;
    int offset;
    int size;
    std::string header;
};

static void fota_recv(void* arg, char* pusrdata, int length)
{
    struct espconn* pespconn = (struct espconn*)arg;
    void** reserve = (void**)pespconn->reverse;
    struct fota_context* context = (struct fota_context*)*reserve;

    if (context == NULL)
    {
        *reserve = context = new fota_context;
        partition_item_t partition_item = {};
        if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
            system_partition_get_item(SYSTEM_PARTITION_OTA_2, &partition_item);
        if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
            system_partition_get_item(SYSTEM_PARTITION_OTA_1, &partition_item);
        context->address = partition_item.addr - 0x1000;
        context->offset = 0;
        context->size = 0;
    }

    if (context->size == 0)
    {
        context->header += std::string(pusrdata, length);
        size_t pos = context->header.find("\r\n\r\n");
        if (pos == std::string::npos)
            return;
        int skip = length - (context->header.length() - (pos + 4));
        pusrdata += skip;
        length -= skip;

        // Content-Length:
        pos = context->header.find("Content-Length: ");
        if (pos != std::string::npos)
            context->size = strtol(context->header.data() + pos + sizeof("Content-Length: ") - 1, 0, 10);
        if (context->size == 0)
        {
            espconn_disconnect(pespconn);
            return;
        }
        context->header = std::string();
        if (length == 0)
            return;
    }

    ETS_GPIO_INTR_DISABLE();
    if (context->offset == 0)
    {
        system_upgrade_init();
        system_upgrade_flag_set(UPGRADE_FLAG_START);
        spi_flash_erase_sector(context->address / SPI_FLASH_SEC_SIZE);
    }
    if (context->offset < 0x1000 && context->offset + length >= 0x1000)
    {
        int skip = 0x1000 - context->offset;
        pusrdata += skip;
        length -= skip;
        context->address += skip;
        context->offset += skip;
        spi_flash_erase_sector(context->address / SPI_FLASH_SEC_SIZE);
    }
    if (context->offset >= 0x1000)
    {
        if (((context->address + length) / SPI_FLASH_SEC_SIZE) != (context->address / SPI_FLASH_SEC_SIZE))
        {
            spi_flash_erase_sector((context->address + length) / SPI_FLASH_SEC_SIZE);
        }
        system_upgrade((uint8*)pusrdata, length);
        os_printf("%d/%d\n", context->offset + length, context->size);
    }
    ETS_GPIO_INTR_ENABLE();
    context->address += length;
    context->offset += length;

    if (context->offset == context->size)
    {
        system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
        espconn_disconnect(pespconn);
    }
}

static void fota_disconn(void* arg)
{
    struct espconn* pespconn = (struct espconn*)arg;
    void** reserve = (void**)pespconn->reverse;
    struct fota_context* context = (struct fota_context*)*reserve;

    delete context;

    if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH)
    {
        system_upgrade_deinit();
        system_upgrade_reboot();
    }
    if (system_upgrade_flag_check() != UPGRADE_FLAG_START)
    {
        system_upgrade_deinit();
    }
}

void fota(const char* url)
{
    https_connect(url, fota_recv, fota_disconn);
}
