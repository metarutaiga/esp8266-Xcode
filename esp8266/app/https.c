#include "eagle.h"
#include <sys/socket.h>
#include <lwip/dns.h>
#define ESP_PLATFORM
#include <utils/common.h>
#include <tls/tls.h>
#include "https.h"

#define TAG __FILE_NAME__

struct https_context
{
    void* other_context;
    void (*disconn)(void* arg);
    void (*recv)(void* arg, char* pusrdata, int length);
    char* host;
    char* path;
    char* attr;
    ip_addr_t ip;
    int socket;
    void* tls;
    struct tls_connection* conn;
    void* temp;
    int need_more_data;
};

static void https_handler(void* arg)
{
    struct https_context* context = arg;

    context->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (context->socket < 0)
        goto final;

    struct sockaddr_in sockaddr = {};
    sockaddr.sin_len = sizeof(sockaddr);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(443);
    sockaddr.sin_addr.s_addr = context->ip.addr;
    if (connect(context->socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
        goto final;

    context->tls = tls_init();
    if (context->tls == NULL)
        goto final;
    context->conn = tls_connection_init(context->tls);
    if (context->conn == NULL)
        goto final;
    context->temp = malloc(1536);
    if (context->temp == NULL)
        goto final;

    // Hello
    struct wpabuf in;
    wpabuf_set(&in, NULL, 0);
    struct wpabuf* out = tls_connection_handshake2(context->tls, context->conn, &in, NULL, &context->need_more_data);

    if (out)
    {
        if (wpabuf_len(out) != 0)
        {
            send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
        }
        wpabuf_free(out);
    }

    // Handshake
    for (;;)
    {
        int length = recv(context->socket, context->temp, 1536, 0);
        if (length < 0)
            goto final;
        if (length == 0)
            goto final;

        struct wpabuf in;
        wpabuf_set(&in, context->temp, length);
        struct wpabuf* out = tls_connection_handshake2(context->tls, context->conn, &in, NULL, &context->need_more_data);

        if (out == NULL)
        {
            if (context->need_more_data == 0)
            {
                ESP_LOGE(TAG, "TLS handshake failed");
                goto final;
            }
        }
        else
        {
            if (tls_connection_get_failed(context->tls, context->conn))
            {
                ESP_LOGE(TAG, "TLS handshake failed");
                goto final;
            }
            if (tls_connection_established(context->tls, context->conn))
            {
                ESP_LOGI(TAG, "TLS handshake established");
                if (wpabuf_len(out) != 0)
                {
                    send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
                }
                wpabuf_free(out);

                sprintf(context->temp,
                        "GET /%s HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "%s%s"
                        "\r\n", context->path, context->host, context->attr ? context->attr : "", context->attr ? "\r\n" : "");
                https_send(context, context->temp, strlen(context->temp));
                break;
            }
            send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
            wpabuf_free(out);
        }
    }

    // Established
    for (;;)
    {
        int length = recv(context->socket, context->temp, 1536, 0);
        if (length < 0)
            goto final;
        if (length == 0)
            goto final;

        struct wpabuf in;
        wpabuf_set(&in, context->temp, length);
        struct wpabuf* out = tls_connection_decrypt2(context->tls, context->conn, &in, NULL);

        if (out)
        {
            if (wpabuf_len(out) != 0)
            {
                context->recv(context, wpabuf_mhead_u8(out), wpabuf_len(out));
            }
            wpabuf_free(out);
        }
    }

final:
    https_disconnect(context);
    vTaskDelete(NULL);
}

static void dns_found(const char* name, const ip_addr_t* ip, void* arg)
{
    struct https_context* context = arg;

    if (ip)
    {
        context->ip = *ip;
        xTaskCreate(&https_handler, "https_handler", 6144, context, 5, NULL);
        return;
    }

    https_disconnect(context);
}

void https_connect(const char* url, const char* attr, void (*recv)(void* arg, char* pusrdata, int length), void (*disconn)(void* arg))
{
    char* buffer = strdup(url);
    if (buffer == NULL)
        return;

    char* token = buffer;
    char* https = strsep(&token, ":/");
    strsep(&token, ":/");
    strsep(&token, ":/");
    char* host = strsep(&token, ":/");
    char* path = token;

    if (strncmp(https, "https", 5) == 0)
    {
        struct https_context* context = calloc(1, sizeof(struct https_context));
        context->disconn = disconn;
        context->recv = recv;
        context->host = strdup(host);
        context->path = strdup(path);
        context->attr = attr ? strdup(attr) : NULL;
        context->socket = -1;
        dns_gethostbyname(context->host, &context->ip, dns_found, context);
    }

    free(buffer);
}

void https_disconnect(void* arg)
{
    struct https_context* context = arg;

    if (context)
    {
        if (context->socket >= 0)
        {
            closesocket(context->socket);
        }
        if (context->disconn)
        {
            context->disconn(arg);
        }
        if (context->tls)
        {
            if (context->conn)
            {
                tls_connection_deinit(context->tls, context->conn);
            }
            tls_deinit(context->tls);
        }
        free(context->path);
        free(context->host);
        free(context->attr);
        free(context->temp);
        free(context);
    }

    ESP_LOGI(TAG, "HTTPS Disconnected");
}

void https_send(void* arg, const void* data, int length)
{
    struct https_context* context = arg;

    struct wpabuf in;
    wpabuf_set(&in, data, length);
    struct wpabuf* out = tls_connection_encrypt(context->tls, context->conn, &in);
    if (out)
    {
        send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
        wpabuf_free(out);
    }
}
