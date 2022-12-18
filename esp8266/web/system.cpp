#include "esp8266.h"
#include <string>
#include "http.h"
#include "fs/fs.h"

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
        if (http_chunk_send(arg, 1, html.data(), html.length()) == false)
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
        html +=     "<input name='ssid' length=32 value='"; html += ssid + "'>";
        html +=     "<br>";
        html +=     "<label>PASS</label>";
        html +=     "<input type='password' name='pass' length=32>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (http_chunk_send(arg, 2, html.data(), html.length()) == false)
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
        html +=     "<input name='ip' length=32 value='"; html += ip + "'>";
        html +=     "<br>";
        html +=     "<label>Gateway</label>";
        html +=     "<input name='gateway' length=32 value='"; html += gateway + "'>";
        html +=     "<br>";
        html +=     "<label>Subnet</label>";
        html +=     "<input name='subnet' length=32 value='"; html += subnet + "'>";
        html +=     "<br>";
        html +=     "<label>DNS</label>";
        html +=     "<input name='dns' length=32 value='"; html += dns + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (http_chunk_send(arg, 3, html.data(), html.length()) == false)
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
        html +=     "<input name='ota' length=32 value='"; html += ota + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (http_chunk_send(arg, 4, html.data(), html.length()) == false)
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
        html +=     "<input name='mqtt' length=32 value='"; html += mqtt + "'>";
        html +=     "<br>";
        html +=     "<label>PORT</label>";
        html +=     "<input name='port' length=32 value='"; html += mqttPort + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        if (http_chunk_send(arg, 5, html.data(), html.length()) == false)
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
        html +=     "<input name='name' length=32 value='"; html += ntp + "'>";
        html +=     "<br>";
        html +=     "<label>ZONE</label>";
        html +=     "<input name='zone' length=32 value='"; html += ntpZone + "'>";
        html +=     "<input type='submit'>";
        html += "</form>";
        html += sntp_get_real_time(sntp_get_current_timestamp());
        if (http_chunk_send(arg, 6, html.data(), html.length()) == false)
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
        if (http_chunk_send(arg, 7, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 7:
        if (http_chunk_send(arg, 8, "", 0) == false)
            return true;
        html.clear();
    default:
        break;
    }

    return false;
}

bool web_ssid(void* arg, const char* url, int line)
{
    struct espconn* pespconn = (struct espconn*)arg;

    char* buffer = strdup(url);
    if (buffer)
    {
        std::string ssid;
        std::string pass;

        char* token = buffer;
        char* path = strsep(&token, "?");
        while (path)
        {
            char* key = strsep(&token, "?=&");
            char* value = strsep(&token, "?=&");
            if (key == nullptr || value == nullptr)
                break;
            if (strcmp(key, "ssid") == 0)
                ssid = value;
            if (strcmp(key, "pass") == 0)
                pass = value;
        }

        int fd = fs_open("ssid", "w");
        if (fd >= 0)
        {
            fs_write(ssid.data(), ssid.length(), fd);
            fs_write("\n", 1, fd);
            fs_write(pass.data(), pass.length(), fd);
            fs_write("\n", 1, fd);
            fs_close(fd);
        }
 
        os_free(buffer);
    }

    char header[128];
    int length = os_sprintf(header,
                            "HTTP/1.1 302 Found\r\n"
                            "Location: %s\r\n"
                            "\r\n", "/");
    espconn_sent(pespconn, (uint8_t*)header, length);
    return false;
}
