#include "esp8266.h"
#include "http.h"

#define HTTP_COPY 0

struct http_handler
{
    struct http_handler* next;
    const char *url;
    const char *type;
    bool (*handler)(void *arg, const char* url, int line);
};
struct http_handler* http_handlers;

struct http_chunk
{
    bool (*handler)(void *arg, const char* url, int line);
    char* url;
    int line;
};

void http_server_recv(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pespconn = arg;

    char *ptr = pusrdata;
    char *method = strsep(&ptr, " ");
    char *url = strsep(&ptr, " ");

    if (method && url && strcmp(method, "GET") == 0)
    {
        struct http_handler *node = http_handlers;
        while (node)
        {
            if (strncmp(node->url, url, strlen(node->url)) == 0)
            {
                struct http_chunk *chunk = pespconn->reverse = os_realloc(pespconn->reverse, sizeof(struct http_chunk));
                chunk->handler = node->handler;
                chunk->url = strdup(url);
                chunk->line = 0;

                if (node->type)
                {
                    char header[128];
                    int length = os_sprintf(header,
                                            "HTTP/1.1 200 OK\r\n"
                                            "Content-Type: %s\r\n"
                                            "Transfer-Encoding: chunked\r\n"
                                            "\r\n", node->type);
                    espconn_sent(pespconn, (uint8_t*)header, length);
                }
                else
                {
                    if (chunk->handler(arg, chunk->url, chunk->line))
                        return;
                    espconn_disconnect(pespconn);
                }
                return;
            }
            node = node->next;
        }
    }
    espconn_disconnect(pespconn);
}

void http_server_sent(void *arg)
{
    struct espconn *pespconn = arg;

    struct http_chunk *chunk = (struct http_chunk *)pespconn->reverse;
    if (chunk->handler(arg, chunk->url, chunk->line))
        return;
    espconn_disconnect(pespconn);
}

void http_server_discon(void *arg)
{
    struct espconn *pespconn = arg;

    struct http_chunk *chunk = (struct http_chunk *)pespconn->reverse;
    if (chunk)
    {
        os_free(chunk->url);
        os_free(chunk);
        pespconn->reverse = NULL;
    }
}

void http_listen(void *arg)
{
    struct espconn *pespconn = arg;

    espconn_regist_recvcb(pespconn, http_server_recv);
    espconn_regist_disconcb(pespconn, http_server_discon);
    if (HTTP_COPY)
    {
        espconn_set_opt(pespconn, ESPCONN_COPY);
        espconn_regist_write_finish(pespconn, http_server_sent);
    }
    else
    {
        espconn_regist_sentcb(pespconn, http_server_sent);
    }
}

void http_init(int port)
{
    static struct espconn esp_conn;
    static esp_tcp esptcp;

    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&esp_conn, http_listen);
    espconn_accept(&esp_conn);
}

void http_regist(const char* url, const char *type, bool (*handler)(void *arg, const char* url, int line))
{
    if (url == NULL || handler == NULL)
        return;

    struct http_handler *node = http_handlers;
    while (node)
    {
        if (strcmp(node->url, url) == 0)
        {
            node->url = url;
            node->type = type;
            node->handler = handler;
            return;
        }
        node = node->next;
    }
    node = os_zalloc(sizeof(struct http_handler));
    node->next = http_handlers;
    node->url = url;
    node->type = type;
    node->handler = handler;
    http_handlers = node;
}

void http_redirect(void* arg, const char* url)
{
    struct espconn *pespconn = arg;

    char header[128];
    int length = os_sprintf(header,
                            "HTTP/1.1 302 Found\r\n"
                            "Location: %s\r\n"
                            "\r\n", url);
    espconn_sent(pespconn, (uint8_t*)header, length);
}

bool http_chunk_send(void *arg, int line, const char *data, size_t data_length)
{
    struct espconn *pespconn = arg;

    char buffer[1024];
    size_t number_length = os_sprintf(buffer, "%x", data_length);
    os_memcpy(buffer + number_length, "\r\n", 2);
    os_memcpy(buffer + number_length + 2, data, data_length);
    os_memcpy(buffer + number_length + 2 + data_length, "\r\n", 2);
    if (espconn_sent(pespconn, (uint8_t*)buffer, number_length + 2 + data_length + 2) != ESPCONN_OK)
    {
        return false;
    }
    struct http_chunk *chunk = (struct http_chunk *)pespconn->reverse;
    chunk->line = line;
    return true;
}

void http_parameter_parse(const char* url, void (*parser)(void* context, const char* key, const char* value), void* context)
{
    char* buffer = strdup(url);
    if (buffer)
    {
        char* token = buffer;
        char* path = strsep(&token, "?=&");
        while (path)
        {
            char* key = strsep(&token, "?=&");
            char* value = strsep(&token, "?=&");
            if (key == NULL || value == NULL)
                break;
            parser(context, key, value);
        }
        os_free(buffer);
    }
}
