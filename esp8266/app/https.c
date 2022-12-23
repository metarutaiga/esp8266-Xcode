#include "esp8266.h"
#undef os_calloc
#undef os_zalloc
#undef os_free
#include <crypto/utils/build_config.h>
#include <crypto/utils/common.h>
#include <crypto/utils/wpabuf.h>
#include <crypto/tls.h>
#define os_zalloc os_zalloc_iram
#define os_calloc os_calloc_iram
#define os_free(s) vPortFree(s, "", __LINE__)
#include "https.h"

struct https_context
{
    void (*recv)(char* pusrdata, int length);
    char* host;
    char* path;
    struct ip_addr ip;
    void* tls;
    struct tls_connection* conn;
    int need_more_data;
};

static void https_discon(void* arg)
{
    struct espconn* pespconn = arg;
    struct https_context* context = pespconn->reverse;

    if (context)
    {
        if (context->tls)
        {
            if (context->conn)
            {
                tls_connection_deinit(context->tls, context->conn);
            }
            tls_deinit(context->tls);
        }
        os_free(context->host);
        os_free(context->path);
        os_free(context);
        pespconn->reverse = NULL;
    }
    os_free(pespconn->proto.tcp);
    os_free(pespconn);
}

static void https_recv(void* arg, char* pusrdata, unsigned short length)
{
    struct espconn* pespconn = arg;
    struct https_context* context = pespconn->reverse;

    struct wpabuf in;
    wpabuf_set(&in, pusrdata, length);
    struct wpabuf* out = tls_connection_decrypt2(context->tls, context->conn, &in, &context->need_more_data);
    if (out)
    {
        context->recv(wpabuf_mhead_u8(out), wpabuf_len(out));
        wpabuf_free(out);
    }
}

static void https_tls_recv(void* arg, char* pusrdata, unsigned short length)
{
    struct espconn* pespconn = arg;
    struct https_context* context = pespconn->reverse;

    struct wpabuf in;
    wpabuf_set(&in, pusrdata, length);
    struct wpabuf* out = tls_connection_handshake2(context->tls, context->conn, &in, NULL, &context->need_more_data);

    if (out == NULL)
    {
        if (context->need_more_data == 0)
        {
            os_printf("%s\n", "TLS handshake failed");
            espconn_disconnect(pespconn);
            return;
        }
    }
    else
    {
        if (tls_connection_get_failed(context->tls, context->conn))
        {
            os_printf("%s\n", "TLS handshake failed");
            espconn_disconnect(pespconn);
            return;
        }
        if (tls_connection_established(context->tls, context->conn))
        {
            os_printf("%s\n", "TLS handshake established");
            wpabuf_free(out);

            char buffer[256];
            wpabuf_set(&in, buffer, os_sprintf(buffer,
                                               "GET /%s HTTP/1.1\r\n"
                                               "Connection: close\r\n"
                                               "Host: %s\r\n"
                                               "\r\n", context->path, context->host));
            out = tls_connection_encrypt(context->tls, context->conn, &in);
            espconn_regist_recvcb(pespconn, https_recv);
            espconn_sent(pespconn, wpabuf_mhead_u8(out), wpabuf_len(out));
            wpabuf_free(out);
            return;
        }
        espconn_regist_recvcb(pespconn, https_tls_recv);
        espconn_sent(pespconn, wpabuf_mhead_u8(out), wpabuf_len(out));
        wpabuf_free(out);
    }
}

static void https_tls_connect(void* arg)
{
    struct espconn* pespconn = arg;
    struct https_context* context = pespconn->reverse;

    struct tls_config conf = {};
    context->tls = tls_init(&conf);
    if (context->tls)
    {
        context->conn = tls_connection_init(context->tls);
        if (context->conn)
        {
            https_tls_recv(pespconn, NULL, 0);
            return;
        }
    }

    espconn_disconnect(pespconn);
}

static void https_dns_found(const char* name, ip_addr_t* ipaddr, void* arg)
{
    struct espconn* pespconn = arg;

    if (ipaddr)
    {
        pespconn->proto.tcp = os_zalloc(sizeof(esp_tcp));
        memcpy(pespconn->proto.tcp->remote_ip, ipaddr, 4);
        pespconn->proto.tcp->remote_port = 443;
        espconn_regist_connectcb(pespconn, https_tls_connect);
        espconn_regist_disconcb(pespconn, https_discon);
        espconn_regist_recvcb(pespconn, https_tls_recv);
        espconn_connect(pespconn);
        return;
    }

    https_discon(pespconn);
}

void https_connect(const char* url, void (*recv)(char* pusrdata, int length))
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
        struct espconn* esp_conn = os_zalloc(sizeof(struct espconn));
        esp_conn->type = ESPCONN_TCP;
        esp_conn->state = ESPCONN_NONE;
        
        struct https_context* context = esp_conn->reverse = os_zalloc(sizeof(struct https_context));
        context->recv = recv;
        context->host = strdup(host);
        context->path = strdup(path);
        espconn_gethostbyname(esp_conn, context->host, &context->ip, https_dns_found);
    }

    os_free(buffer);
}
