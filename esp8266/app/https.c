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
    TimerHandle_t timer;
    void (*disconn)(void* arg);
    void (*recv)(void* arg, char* pusrdata, int length);
    char* host;
    char* path;
    ip_addr_t ip;
    int socket;
    void* tls;
    struct tls_connection* conn;
    void* temp;
    int need_more_data;
    int established;
};

static void https_handler(TimerHandle_t timer)
{
    struct https_context* context = pvTimerGetTimerID(timer);

    if (context->socket == -1)
    {
        context->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        int mode = 1;
        ioctlsocket(context->socket, FIONBIO, &mode);

        struct sockaddr_in sockaddr = {};
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(443);
        sockaddr.sin_addr.s_addr = context->ip.addr;
        connect(context->socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    }
    else if (context->tls == NULL)
    {
        context->tls = tls_init();
        if (context->tls)
        {
            context->conn = tls_connection_init(context->tls);
            if (context->conn)
            {
                context->temp = malloc(1536);

                struct wpabuf in;
                wpabuf_set(&in, NULL, 0);
                struct wpabuf* out = tls_connection_handshake2(context->tls, context->conn, &in, NULL, &context->need_more_data);
                if (out)
                {
                    send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
                    wpabuf_free(out);
                }

                xTimerChangePeriod(timer, 10 / portTICK_PERIOD_MS, 0);
                return;
            }
        }
        https_disconnect(context);
    }
    else if (context->established == 0)
    {
        int length = recv(context->socket, context->temp, 1536, 0);
        if (length < 0)
            return;
        if (length == 0)
        {
            https_disconnect(context);
            return;
        }

        struct wpabuf in;
        wpabuf_set(&in, context->temp, length);
        struct wpabuf* out = tls_connection_handshake2(context->tls, context->conn, &in, NULL, &context->need_more_data);

        if (out == NULL)
        {
            if (context->need_more_data == 0)
            {
                ESP_LOGE(TAG, "TLS handshake failed");
                https_disconnect(context);
                return;
            }
        }
        else
        {
            if (tls_connection_get_failed(context->tls, context->conn))
            {
                ESP_LOGE(TAG, "TLS handshake failed");
                https_disconnect(context);
                return;
            }
            if (tls_connection_established(context->tls, context->conn))
            {
                ESP_LOGI(TAG, "TLS handshake established");
                wpabuf_free(out);

                sprintf(context->temp,
                        "GET /%s HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "\r\n", context->path, context->host);
                https_send(context, context->temp, strlen(context->temp));
                context->established = 1;
                return;
            }
            send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
            wpabuf_free(out);
        }
    }
    else
    {
        int length = recv(context->socket, context->temp, 1536, 0);
        if (length < 0)
            return;
        if (length == 0)
        {
            https_disconnect(context);
            return;
        }

        struct wpabuf in;
        wpabuf_set(&in, context->temp, length);
        struct wpabuf* out = tls_connection_decrypt2(context->tls, context->conn, &in, &context->need_more_data);
        if (out)
        {
            if (wpabuf_len(out) != 0)
            {
                context->recv(context, wpabuf_mhead_u8(out), wpabuf_len(out));
            }
            wpabuf_free(out);
        }
    }
}

static void dns_found(const char* name, const ip_addr_t* ip, void* arg)
{
    struct https_context* context = arg;

    context->ip = *ip;
    context->timer = xTimerCreate("HTTPS", 1000 / portTICK_PERIOD_MS, pdTRUE, context, https_handler);
    xTimerStart(context->timer, 0);
}

void https_connect(const char* url, void (*recv)(void* arg, char* pusrdata, int length), void (*disconn)(void* arg))
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
        xTimerDelete(context->timer, 0);
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
        free(context->host);
        free(context->path);
        free(context->temp);
        free(context);
    }

    ESP_LOGE(TAG, "Disconnect");
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
