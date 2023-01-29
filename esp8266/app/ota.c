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

#if 0
static void ota_tcp_recv(void* arg, char* pusrdata, unsigned short length)
{
    struct espconn* pespconn = arg;
    struct ota_context* context = pespconn->reverse;
 
    ETS_GPIO_INTR_DISABLE();
    if (context->offset == 0)
    {
        system_upgrade_init();
        system_upgrade_flag_set(UPGRADE_FLAG_START);
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
        system_upgrade(pusrdata, length);
        os_printf("%d/%d\n", context->offset + length, context->size);
    }
    ETS_GPIO_INTR_ENABLE();
    context->address += length;
    context->offset += length;
    espconn_sent(pespconn, "OK", 2);
}

static void ota_tcp_send(void* arg)
{
    struct espconn* pespconn = arg;
    struct ota_context* context = pespconn->reverse;

    if (context->offset == context->size)
    {
        system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
        espconn_sent(pespconn, "OK", 2);
        espconn_disconnect(pespconn);
    }
}

static void ota_tcp_discon(void* arg)
{
    struct espconn* pespconn = arg;

    os_free(pespconn->reverse);
    os_free(pespconn);

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

static void ota_tcp_init(char ip[4], int port, int size)
{
    struct espconn* esp_conn = os_zalloc(sizeof(struct espconn));
    struct ota_context* context = os_zalloc(sizeof(struct ota_context));
    esp_conn->type = ESPCONN_TCP;
    esp_conn->state = ESPCONN_NONE;
    esp_conn->proto.tcp = os_zalloc(sizeof(esp_tcp));
    os_memcpy(esp_conn->proto.tcp->remote_ip, ip, 4);
    esp_conn->proto.tcp->remote_port = port;
    esp_conn->reverse = context;
    partition_item_t partition_item = {};
    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
        system_partition_get_item(SYSTEM_PARTITION_OTA_2, &partition_item);
    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
        system_partition_get_item(SYSTEM_PARTITION_OTA_1, &partition_item);
    context->address = partition_item.addr - 0x1000;
    context->offset = 0;
    context->size = size;
    espconn_regist_recvcb(esp_conn, ota_tcp_recv);
    espconn_regist_sentcb(esp_conn, ota_tcp_send);
    espconn_regist_disconcb(esp_conn, ota_tcp_discon);
    espconn_connect(esp_conn);
}

void ota_udp_recv(void* arg, char* pusrdata, unsigned short length)
{
    struct espconn* pespconn = arg;

    char* buffer = strdup(pusrdata);
    if (buffer == NULL)
        return;
    const char* command = strsep(&buffer, " ");
    const char* remote_port = strsep(&buffer, " ");
    const char* content_size = strsep(&buffer, " ");
    const char* file_md5 = strsep(&buffer, " ");
    if (command && remote_port && content_size && file_md5)
    {
        remot_info* remote = NULL;
        if (espconn_get_connection_info(pespconn, &remote, 0) == 0)
        {
            os_memcpy(pespconn->proto.udp->remote_ip, remote->remote_ip, 4);
            pespconn->proto.udp->remote_port = remote->remote_port;
            espconn_sent(pespconn, "OK", 2);

            ota_tcp_init(remote->remote_ip, strtol(remote_port, NULL, 10), strtol(content_size, NULL, 10));
        }
    }

    os_free(buffer);
}
#endif

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
                switch (GET_PERI_REG_BITS(CACHE_FLASH_CTRL_REG, 25, 24))
                {
                case 0:
                    ESP_LOGI(TAG, "USER2");
                    spi_flash_erase_sector(0x3FD000 / SPI_FLASH_SEC_SIZE);
                    spi_flash_write(0x3FD000, "\xFD\xE7", 2);
                    spi_flash_write(0x3FF000, "", 1);
                    break;
                case 2:
                    ESP_LOGI(TAG, "USER1");
                    spi_flash_erase_sector(0x3FD000 / SPI_FLASH_SEC_SIZE);
                    spi_flash_write(0x3FD000, "\xFC\xE7", 2);
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
                context->size = strtol(content_size, NULL, 10);
                context->offset = 0;
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
