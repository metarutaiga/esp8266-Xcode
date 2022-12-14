#include "esp8266.h"
#include <stdlib.h>
#include <spi_flash.h>
#include <upgrade.h>
#include "ota.h"

#define espconn_sent(espconn, psent, length) \
{ \
    const char* rom = psent; \
    uint8_t ram[length]; \
    for (int i = 0; i < length; ++i) \
    { \
        ram[i] = pgm_read_byte(rom + i); \
    } \
    espconn_sent(espconn, ram, length); \
}

struct ota_context
{
    int address;
    int offset;
    int size;
};

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

void ota_init(int port)
{
    static struct espconn* esp_conn IRAM_ATTR;
    if (esp_conn == NULL)
    {
        esp_conn = os_zalloc(sizeof(struct espconn));
        esp_conn->type = ESPCONN_UDP;
        esp_conn->state = ESPCONN_NONE;
        esp_conn->proto.udp = os_zalloc(sizeof(esp_udp));
        esp_conn->proto.udp->local_port = port;
        espconn_regist_recvcb(esp_conn, ota_udp_recv);
        espconn_create(esp_conn);
    }
}
