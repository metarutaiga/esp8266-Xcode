#include "eagle.h"
#include <string>
#include <spi_flash.h>
#include "https.h"
#include "fota.h"

#define TAG __FILE_NAME__

struct fota_context
{
    int address;
    int offset;
    int size;
    void* temp = nullptr;
    string header;
};

static void fota_recv(void* arg, char* data, int length)
{
    void** other_context = (void**)arg;
    struct fota_context* context = (struct fota_context*)*other_context;

    if (context == NULL)
    {
        *other_context = context = new fota_context;

        uint32_t sub_region = GET_PERI_REG_BITS(CACHE_FLASH_CTRL_REG, 25, 24);

        context->address = (sub_region == 0) ? 0x100000 : 0x000000;
        context->offset = 0;
        context->size = 0;
        context->temp = realloc(context->temp, 1536);
    }

    if (context->size == 0)
    {
        context->header += string(data, length);
        size_t pos = context->header.find(string("\r\n\r\n"));
        if (pos == string::npos)
            return;
        int skip = length - (context->header.length() - (pos + 4));
        data += skip;
        length -= skip;

        // Content-Length:
        pos = context->header.find(string("Content-Length: "));
        if (pos != string::npos)
            context->size = strtol(context->header.data() + pos + sizeof("Content-Length: ") - 1, 0, 10);
        if (context->size == 0)
        {
            https_disconnect(arg);
            return;
        }
        context->header = string();
        if (length == 0)
            return;
    }

    vPortETSIntrLock();
    if (context->offset < 0x1000 && context->offset + length >= 0x1000)
    {
        int skip = 0x1000 - context->offset;
        data += skip;
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
        spi_flash_write(context->address, data, length);
        ESP_LOGI(TAG, "%d/%d", context->offset + length, context->size);
    }
    vPortETSIntrUnlock();
    context->address += length;
    context->offset += length;
    if (context->offset == context->size)
    {
        uint32_t boot_param = 0xFFFFE7FC;
        switch (GET_PERI_REG_BITS(CACHE_FLASH_CTRL_REG, 25, 24))
        {
        case 0:
            ESP_LOGI(TAG, "USER2");
            boot_param |= 0x01;
            spi_flash_erase_sector(0x3FD000 / SPI_FLASH_SEC_SIZE);
            spi_flash_write(0x3FD000, &boot_param, sizeof(boot_param));
            spi_flash_write(0x3FF000, "", 1);
            break;
        case 2:
            ESP_LOGI(TAG, "USER1");
            boot_param &= ~0x01;
            spi_flash_erase_sector(0x3FD000 / SPI_FLASH_SEC_SIZE);
            spi_flash_write(0x3FD000, &boot_param, sizeof(boot_param));
            spi_flash_write(0x3FF000, "", 1);
            break;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_reset(ESP_RST_SW);
    }
}

static void fota_disconn(void* arg)
{
    void** other_context = (void**)arg;
    struct fota_context* context = (struct fota_context*)*other_context;

    delete context;
}

void fota(const char* url)
{
    https_connect(url, nullptr, fota_recv, fota_disconn);
}

void fota_callback(void* arg)
{
    https_callback(arg, fota_recv, fota_disconn);
}
