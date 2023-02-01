#include "eagle.h"
#include <sys/socket.h>
#include <esp_http_server.h>

#define HTTPD_STACK_SIZE 2048
#define HTTPD_MAX_CONNECTIONS 16

#define TAG __FILE_NAME__

struct httpd_uri_node
{
    struct httpd_uri_node* next;
    httpd_uri_t uri_handler;
};
static struct httpd_uri_node* uri_node IRAM_ATTR;

static void httpd_handler(void* arg)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd < 0)
        goto final;

    struct sockaddr_in sockaddr = {};
    sockaddr.sin_len = sizeof(sockaddr);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(80);
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listen_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
        goto final;
    if (listen(listen_fd, HTTPD_MAX_CONNECTIONS) < 0)
        goto final;

    int fds[HTTPD_MAX_CONNECTIONS];
    httpd_req_t* reqs[HTTPD_MAX_CONNECTIONS] = {};
    memset(fds, 0xFF, sizeof(fds));

    for (;;)
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(listen_fd, &set);

        int max_fd = listen_fd;
        for (int i = 0; i < HTTPD_MAX_CONNECTIONS; ++i)
        {
            int fd = fds[i];
            if (fd >= 0)
            {
                FD_SET(fd, &set);
                if (max_fd < fd)
                {
                    max_fd = fd;
                }
            }
        }

        if (select(max_fd + 1, &set, NULL, NULL, NULL) <= 0)
            continue;

        if (FD_ISSET(listen_fd, &set))
        {
            socklen_t sockaddr_len = sizeof(sockaddr);
            int fd = accept(listen_fd, (struct sockaddr*)&sockaddr, &sockaddr_len);
            if (fd >= 0)
            {
                for (int i = 0; i < HTTPD_MAX_CONNECTIONS; ++i)
                {
                    if (fds[i] < 0)
                    {
                        fds[i] = fd;
                        reqs[i] = calloc(1, sizeof(httpd_req_t));
                        fd = -1;
                        break;
                    }
                }
                if (fd >= 0)
                {
                    closesocket(fd);
                    ESP_LOGE(TAG, "httpd is full (%d)", fd);
                }
            }
        }

        char* buf = malloc(1536);
        for (int i = 0; i < HTTPD_MAX_CONNECTIONS; ++i)
        {
            int fd = fds[i];
            if (fd >= 0)
            {
                if (FD_ISSET(fd, &set))
                {
                    bool closed = false;
                    int length = recv(fd, buf, 1536, 0);
                    if (length <= 0)
                    {
                        closed = true;
                    }
                    else
                    {
                        httpd_req_t* req = reqs[i];
                        if (req->sess_ctx == NULL)
                        {
                            if (strncmp(buf, "GET", 3) != 0)
                            {
                                closed = true;
                            }
                            else
                            {
                                req->method = fd;
                                for (int i = 0; i < HTTPD_MAX_URI_LEN; ++i)
                                {
                                    char c = buf[i + 4];
                                    if (c == 0 || c == ' ')
                                        break;
                                    ((char*)req->uri)[i] = c;
                                }
                                struct httpd_uri_node* node = uri_node;
                                while (node)
                                {
                                    if (strncmp(req->uri, node->uri_handler.uri, strlen(node->uri_handler.uri)) == 0)
                                    {
                                        req->sess_ctx = &node->uri_handler;
                                        break;
                                    }
                                    node = node->next;
                                }
                            }
                        }
                        if (req->sess_ctx == NULL)
                        {
                            closed = true;
                        }
                        if (req->sess_ctx)
                        {
                            httpd_uri_t* uri = req->sess_ctx;
                            if (uri->handler(req) != ESP_FAIL)
                            {
                                closed = true;
                            }
                        }
                    }
                    if (closed)
                    {
                        closesocket(fd);
                        free(reqs[i]);
                        fds[i] = -1;
                        reqs[i] = NULL;
                        ESP_LOGI(TAG, "%d is disconnected", fd);
                    }
                }
            }
        }
        free(buf);
    }

final:
    for (int i = 0; i < HTTPD_MAX_CONNECTIONS; ++i)
    {
        int fd = fds[i];
        if (fd >= 0)
        {
            closesocket(fd);
        }
        httpd_req_t* req = reqs[i];
        if (req)
        {
            free(req->user_ctx);
            free(req);
        }
    }
    if (listen_fd >= 0)
    {
        closesocket(listen_fd);
    }
    vTaskDelete(NULL);
}

esp_err_t httpd_start(httpd_handle_t* handle, const httpd_config_t* config)
{
    (*handle) = (httpd_handle_t)xTaskCreate(httpd_handler, "httpd", HTTPD_STACK_SIZE, NULL, 5, NULL);
    return ESP_OK;
}

esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t* uri_handler)
{
    struct httpd_uri_node* node = uri_node;
    while (node)
    {
        if (strcmp(node->uri_handler.uri, uri_handler->uri) == 0)
        {
            memcpy(&node->uri_handler, uri_handler, sizeof(httpd_uri_t));
            return ESP_OK;
        }
        node = node->next;
    }
    node = malloc(sizeof(struct httpd_uri_node));
    node->next = uri_node;
    uri_node = node;
    memcpy(&node->uri_handler, uri_handler, sizeof(httpd_uri_t));
    if (node->uri_handler.user_ctx == NULL)
    {
        node->uri_handler.user_ctx = "text/html";
    }
    return ESP_OK;
}

size_t httpd_req_get_url_query_len(httpd_req_t* r)
{
    return strlen(r->uri);
}

esp_err_t httpd_req_get_url_query_str(httpd_req_t* r, char* buf, size_t buf_len)
{
    strncpy(buf, r->uri, buf_len);
    return ESP_OK;
}

esp_err_t httpd_query_key_value(const char* qry, const char* key, char* val, size_t val_size)
{
    char* temp = strdup(qry);
    if (temp)
    {
        char* token = temp;
        char* path = strsep(&token, "?=&");
        while (path)
        {
            char* left = strsep(&token, "?=&");
            char* right = strsep(&token, "?=&");
            if (left == NULL)
                break;
            if (strcmp(left, key) != 0)
                continue;
            if (right == NULL)
            {
                right = "";
            }
            strncpy(val, right, val_size);
        }
        free(temp);
    }
    return ESP_OK;
}

esp_err_t httpd_resp_send(httpd_req_t* r, const char* buf, ssize_t buf_len)
{
    if (r->content_len == 0)
    {
        char header[128];
        int length = sprintf(header,
                             "HTTP/1.1 %s\r\n"
                             "%s%s"
                             "Content-Type: %s\r\n"
                             "\r\n",
                             r->aux ? (char*)r->aux : "200 OK",
                             r->user_ctx ? (char*)r->user_ctx : "",
                             r->user_ctx ? "\r\n" : "",
                             (char*)((httpd_uri_t*)r->sess_ctx)->user_ctx);
        send(r->method, header, length, 0);
        r->content_len += length;
    }
    if (buf == NULL || buf_len == 0)
    {
        send(r->method, NULL, 0, 0);
        return ESP_OK;
    }
    while (buf_len)
    {
        int length = send(r->method, buf, buf_len, 0);
        if (length < 0)
        {
            return ESP_FAIL;
        }
        buf += length;
        buf_len -= length;
        r->content_len += length;
    }
    return ESP_OK;
}

esp_err_t httpd_resp_send_chunk(httpd_req_t* r, const char* buf, ssize_t buf_len)
{
    if (r->content_len == 0)
    {
        char header[128];
        int length = sprintf(header,
                             "HTTP/1.1 %s\r\n"
                             "%s%s"
                             "Content-Type: %s\r\n"
                             "Transfer-Encoding: chunked\r\n"
                             "\r\n",
                             r->aux ? (char*)r->aux : "200 OK",
                             r->user_ctx ? (char*)r->user_ctx : "",
                             r->user_ctx ? "\r\n" : "",
                             (char*)((httpd_uri_t*)r->sess_ctx)->user_ctx);
        send(r->method, header, length, 0);
        r->content_len += length;
    }
    if (buf == NULL || buf_len == 0)
    {
        send(r->method, NULL, 0, 0);
        return ESP_OK;
    }
    char* temp = malloc(buf_len + 16);
    size_t number_length = sprintf(temp, "%x", buf_len);
    memcpy(temp + number_length, "\r\n", 2);
    memcpy(temp + number_length + 2, buf, buf_len);
    memcpy(temp + number_length + 2 + buf_len, "\r\n", 2);
    buf = temp;
    buf_len = number_length + 2 + buf_len + 2;
    while (buf_len)
    {
        int length = send(r->method, buf, buf_len, 0);
        if (length < 0)
        {
            return ESP_FAIL;
        }
        buf += length;
        buf_len -= length;
        r->content_len += length;
    }
    free(temp);
    return ESP_OK;
}

esp_err_t httpd_resp_set_status(httpd_req_t* r, const char* status)
{
    r->aux = (void*)status;
    return ESP_OK;
}

esp_err_t httpd_resp_set_hdr(httpd_req_t* r, const char* field, const char* value)
{
    size_t length = strlen(field) + sizeof(": ") - 1 + strlen(value) + 1;
    void* buffer = r->user_ctx = realloc(r->user_ctx, length);
    if (buffer)
    {
        sprintf(buffer, "%s: %s", field, value);
    }
    return ESP_OK;
}
