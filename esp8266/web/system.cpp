#include "esp8266.h"
#include <string>
#include "app/fs.h"
#include "app/httpd.h"

bool web_system(void *arg, const char* url, int line)
{
    std::string html;

    switch (line)
    {
    case 0:
    {
        // Head
        html += "<html>";
        html += "<head>";
        html +=     "<title>";
        html +=         wifi_station_get_hostname();
        html +=     "</title>";
        html += "</head>";
        html += "<body>";
        if (httpd_chunk_send(arg, 1, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 1:
    {
        // SSID
        std::string ssid;
        int fd = fs_open("ssid", "r");
        if (fd >= 0)
        {
            ssid = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        html += "<form method='get' action='ssid'>";
        html +=     "<label>SSID</label>";
        html +=     "<input name='ssid' length=32 value='" + ssid + "'>";
        html +=     "<br>";
        html +=     "<label>PASS</label>";
        html +=     "<input type='password' name='pass' length=32>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (httpd_chunk_send(arg, 2, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 2:
    {
        struct ip_info info = {};
        wifi_get_ip_info(STATION_IF, &info);
        ip_addr_t dns_server = espconn_dns_getserver(0);

        // IP
        std::string ip;
        std::string gateway;
        std::string subnet;
        std::string dns;
        os_sprintf(number, IPSTR, IP2STR(&info.ip)); ip = number;
        os_sprintf(number, IPSTR, IP2STR(&info.gw)); gateway = number;
        os_sprintf(number, IPSTR, IP2STR(&info.netmask)); subnet = number;
        os_sprintf(number, IPSTR, IP2STR(&dns_server)); dns = number;
        int fd = fs_open("ip", "r");
        if (fd >= 0)
        {
            ip = fs_gets(number, 128, fd);
            gateway = fs_gets(number, 128, fd);
            subnet = fs_gets(number, 128, fd);
            dns = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        html += "<form method='get' action='ip'>";
        html +=     "<label>IP</label>";
        html +=     "<input name='ip' length=32 value='" + ip + "'>";
        html +=     "<br>";
        html +=     "<label>Gateway</label>";
        html +=     "<input name='gateway' length=32 value='" + gateway + "'>";
        html +=     "<br>";
        html +=     "<label>Subnet</label>";
        html +=     "<input name='subnet' length=32 value='" + subnet + "'>";
        html +=     "<br>";
        html +=     "<label>DNS</label>";
        html +=     "<input name='dns' length=32 value='" + dns + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (httpd_chunk_send(arg, 3, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 3:
    {
        // OTA
        std::string ota;
        int fd = fs_open("ota", "r");
        if (fd >= 0)
        {
            ota = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        html += "<form method='get' action='ota'>";
        html +=     "<label>OTA</label>";
        html +=     "<input name='ota' length=32 value='" + ota + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (httpd_chunk_send(arg, 4, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 4:
    {
        // MQTT
        std::string mqtt;
        std::string mqttPort = "1883";
        int fd = fs_open("mqtt", "r");
        if (fd >= 0)
        {
            mqtt = fs_gets(number, 128, fd);
            mqttPort = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        html += "<form method='get' action='mqtt'>";
        html +=     "<label>MQTT</label>";
        html +=     "<input name='mqtt' length=32 value='" + mqtt + "'>";
        html +=     "<br>";
        html +=     "<label>PORT</label>";
        html +=     "<input name='port' length=32 value='" + mqttPort + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (httpd_chunk_send(arg, 5, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 5:
    {
        // NTP
        std::string ntp = "pool.ntp.org";
        std::string ntpZone = "8";
        int fd = fs_open("ntp", "r");
        if (fd >= 0)
        {
            ntp = fs_gets(number, 128, fd);
            ntpZone = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        html += "<form method='get' action='ntp'>";
        html +=     "<label>NTP</label>";
        html +=     "<input name='name' length=32 value='" + ntp + "'>";
        html +=     "<br>";
        html +=     "<label>ZONE</label>";
        html +=     "<input name='zone' length=32 value='" + ntpZone + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        html += sntp_get_real_time(sntp_get_current_timestamp());
        if (httpd_chunk_send(arg, 6, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 6:
    {
        // Reset
        html += "<form method='get' action='reset'>";
        html +=     "<button type='submit'>Reset</button>";
        html += "</form>";

        // Tail
        html += "</body>";
        html += "</html>";
        if (httpd_chunk_send(arg, 7, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 7:
        if (httpd_chunk_send(arg, 8, "", 0) == false)
            return true;
        html.clear();
    default:
        break;
    }

    return false;
}

bool web_ssid(void* arg, const char* url, int line)
{
    httpd_redirect(arg, "/");

    std::string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        std::string& text = *(std::string*)context;
        if (strcmp(key, "ssid") == 0 ||
            strcmp(key, "pass") == 0)
        {
            text += value;
            text += '\n';
        }
    }, &text);

    int fd = fs_open("ssid", "w");
    if (fd >= 0)
    {
        fs_write(text.data(), text.length(), fd);
        fs_close(fd);
    }

    return false;
}

bool web_ip(void* arg, const char* url, int line)
{
    httpd_redirect(arg, "/");

    std::string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        std::string& text = *(std::string*)context;
        if (strcmp(key, "ip") == 0 ||
            strcmp(key, "gateway") == 0 ||
            strcmp(key, "subnet") == 0 ||
            strcmp(key, "dns") == 0)
        {
            text += value;
            text += '\n';
        }
    }, &text);

    int fd = fs_open("ip", "w");
    if (fd >= 0)
    {
        fs_write(text.data(), text.length(), fd);
        fs_close(fd);
    }

    return false;
}

bool web_ota(void* arg, const char* url, int line)
{
    httpd_redirect(arg, "/");

    std::string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        std::string& text = *(std::string*)context;
        if (strcmp(key, "ota") == 0)
        {
            text += value;
            text += '\n';
        }
    }, &text);

    int fd = fs_open("ota", "w");
    if (fd >= 0)
    {
        fs_write(text.data(), text.length(), fd);
        fs_close(fd);
    }

    return false;
}

bool web_mqtt(void* arg, const char* url, int line)
{
    httpd_redirect(arg, "/");

    std::string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        std::string& text = *(std::string*)context;
        if (strcmp(key, "mqtt") == 0 ||
            strcmp(key, "port") == 0)
        {
            text += value;
            text += '\n';
        }
    }, &text);

    int fd = fs_open("mqtt", "w");
    if (fd >= 0)
    {
        fs_write(text.data(), text.length(), fd);
        fs_close(fd);
    }

    return false;
}

bool web_ntp(void* arg, const char* url, int line)
{
    httpd_redirect(arg, "/");

    std::string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        std::string& text = *(std::string*)context;
        if (strcmp(key, "name") == 0 ||
            strcmp(key, "zone") == 0)
        {
            text += value;
            text += '\n';
        }
    }, &text);

    int fd = fs_open("ntp", "w");
    if (fd >= 0)
    {
        fs_write(text.data(), text.length(), fd);
        fs_close(fd);
    }

    return false;
}

bool web_reset(void* arg, const char* url, int line)
{
    httpd_redirect(arg, "/");

    os_timer_t* timer = (os_timer_t*)os_zalloc(sizeof(os_timer_t));
    os_timer_setfn(timer, [](void* arg)
    {
        system_restart_local();
    }, nullptr);
    os_timer_arm(timer, 100, true);

    return false;
}
