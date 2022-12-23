#include "esp8266.h"
#include <stdio.h>
#undef os_calloc
#undef os_zalloc
#define __must_check
#include <crypto/tls.h>
#include <crypto/utils/common.h>
#include <crypto/utils/wpabuf.h>
#define os_zalloc os_zalloc_iram
#define os_calloc os_calloc_iram
#include "https.h"

struct https_handler
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
    struct https_handler* handler = pespconn->reverse;

    if (handler)
    {
        if (handler->tls)
        {
            if (handler->conn)
            {
                tls_connection_deinit(handler->tls, handler->conn);
            }
            tls_deinit(handler->tls);
        }
        os_free(handler->host);
        os_free(handler->path);
        os_free(handler);
        pespconn->reverse = NULL;
    }
    os_free(pespconn->proto.tcp);
    os_free(pespconn);
}

static void https_recv(void* arg, char* pusrdata, unsigned short length)
{
    struct espconn* pespconn = arg;
    struct https_handler* handler = pespconn->reverse;

    struct wpabuf in;
    wpabuf_set(&in, pusrdata, length);
    struct wpabuf* out = tls_connection_decrypt2(handler->tls, handler->conn, &in, &handler->need_more_data);
    if (out)
    {
        handler->recv(wpabuf_mhead_u8(out), wpabuf_len(out));
        wpabuf_free(out);
    }
}

static void https_tls_recv(void* arg, char* pusrdata, unsigned short length)
{
    struct espconn* pespconn = arg;
    struct https_handler* handler = pespconn->reverse;

    struct wpabuf in;
    wpabuf_set(&in, pusrdata, length);
    struct wpabuf* out = tls_connection_handshake2(handler->tls, handler->conn, &in, NULL, &handler->need_more_data);

    if (out == NULL)
    {
        if (handler->need_more_data == 0)
        {
            os_printf("%s\n", "TLS handshake failed");
            espconn_disconnect(pespconn);
            return;
        }
    }
    else
    {
        if (tls_connection_get_failed(handler->tls, handler->conn))
        {
            os_printf("%s\n", "TLS handshake failed");
            espconn_disconnect(pespconn);
            return;
        }
        if (tls_connection_established(handler->tls, handler->conn))
        {
            os_printf("%s\n", "TLS handshake established");
            wpabuf_free(out);

            char buffer[256];
            wpabuf_set(&in, buffer, os_sprintf(buffer,
                                               "GET /%s HTTP/1.1\r\n"
                                               "Connection: close\r\n"
                                               "Host: %s\r\n"
                                               "\r\n", handler->path, handler->host));
            out = tls_connection_encrypt(handler->tls, handler->conn, &in);
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
    struct https_handler* handler = pespconn->reverse;

    struct tls_config conf = {};
    handler->tls = tls_init(&conf);
    if (handler->tls)
    {
        handler->conn = tls_connection_init(handler->tls);
        if (handler->conn)
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
    char* https = strsep(&token, "/");
    strsep(&token, "/");
    char* host = strsep(&token, "/");
    char* path = token;

    struct espconn* esp_conn = os_zalloc(sizeof(struct espconn));
    esp_conn->type = ESPCONN_TCP;
    esp_conn->state = ESPCONN_NONE;

    struct https_handler* handler = esp_conn->reverse = os_zalloc(sizeof(struct https_handler));
    handler->recv = recv;
    handler->host = strdup(host);
    handler->path = strdup(path);
    espconn_gethostbyname(esp_conn, handler->host, &handler->ip, https_dns_found);

    os_free(buffer);
}
