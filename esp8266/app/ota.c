#include "eagle.h"
#include <sys/socket.h>
#include <spi_flash.h>
#include <esp8266/rom_functions.h>
#include "ota.h"

#define TAG __FILE_NAME__

struct ota_context
{
    int udp_socket;
    int tcp_socket;
    int address;
    int offset;
    int size;
    void* temp;
};
static struct ota_context* context IRAM_BSS_ATTR;

static void ota_handler(TimerHandle_t timer)
{
    if (context->tcp_socket >= 0)
    {
        char* data = context->temp;
        int length = recv(context->tcp_socket, data, 1536, 0);
        if (length < 0)
        {
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
        if (length > 0 && length < 1536)
        {
            send(context->tcp_socket, "OK", 2, 0);
        }
        if (length == 0 || context->offset == context->size)
        {
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

                send(context->tcp_socket, "OK", 2, 0);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                esp_reset(ESP_RST_SW);
            }
            close(context->tcp_socket);
            context->tcp_socket = -1;

            free(context->temp);
            context->temp = NULL;

            xTimerChangePeriod(timer, 1000 / portTICK_PERIOD_MS, 0);
        }
    }
    else if (context->udp_socket >= 0)
    {
        char data[256];
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        if (recvfrom(context->udp_socket, data, 256, 0, (struct sockaddr*)&from, &fromlen) > 0)
        {
            char* buffer = data;
            const char* command = strsep(&buffer, " \n");
            const char* remote_port = strsep(&buffer, " \n");
            const char* content_size = strsep(&buffer, " \n");
            const char* file_md5 = strsep(&buffer, " \n");
            if (command && remote_port && content_size && file_md5)
            {
                ESP_LOGI(TAG, "Command: %s", command);
                ESP_LOGI(TAG, "Remote port: %s", remote_port);
                ESP_LOGI(TAG, "Context size: %s", content_size);
                ESP_LOGI(TAG, "File MD5: %s", file_md5);

                uint32_t sub_region = GET_PERI_REG_BITS(CACHE_FLASH_CTRL_REG, 25, 24);

                context->address = (sub_region == 0) ? 0x100000 : 0x000000;
                context->offset = 0;
                context->size = strtol(content_size, NULL, 10);
                context->temp = realloc(context->temp, 1536);

                context->tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

                int mode = 1;
                ioctlsocket(context->tcp_socket, FIONBIO, &mode);

                struct sockaddr_in sockaddr = {};
                sockaddr.sin_family = AF_INET;
                sockaddr.sin_port = htons(strtol(remote_port, NULL, 10));
                sockaddr.sin_addr = ((struct sockaddr_in*)&from)->sin_addr;
                connect(context->tcp_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

                sendto(context->udp_socket, "OK", 2, 0, (struct sockaddr*)&from, fromlen);

                xTimerChangePeriod(timer, 10 / portTICK_PERIOD_MS, 0);
            }
        }
    }
}

void ota_init(int port)
{
    if (context == NULL)
    {
        context = os_zalloc(sizeof(struct ota_context));
        context->udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        context->tcp_socket = -1;

        int mode = 1;
        ioctlsocket(context->udp_socket, FIONBIO, &mode);

        struct sockaddr_in sockaddr = {};
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(port);
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind(context->udp_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

        TimerHandle_t timer = xTimerCreate("OTA", 1000 / portTICK_PERIOD_MS, pdTRUE, (void*)"OTA", ota_handler);
        xTimerStart(timer, 0);
    }
}
