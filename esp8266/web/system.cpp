#include "esp8266.h"
#include <string>
#include "app/fs.h"
#include "app/httpd.h"

bool web_system(void* arg, const char* url, int line)
{
    string html;

    switch (line)
    {
    case 0:
    {
        // Head
        html += "<html>";
        html += "<head>";
        html +=     wifi_station_get_connect_status() == STATION_GOT_IP ? web_css : "";
        html +=     "<title>";
        html +=         wifi_station_get_hostname();
        html +=     "</title>";
        html += "</head>";
        html += "<body>";
        html += "<table>";
        if (httpd_chunk_send(arg, 1, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 1:
    {
        // SSID
        string ssid;
        int fd = fs_open("ssid", "r");
        if (fd >= 0)
        {
            ssid = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        html += "<form method='get' action='ssid'>";
        html +=     "<tr>";
        html +=         "<td>";
        html +=             "<label>SSID</label>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input name='ssid' length=32 value='" + ssid + "'>";
        html +=         "</td>";
        html +=     "</tr>";
        html +=     "<tr>";
        html +=         "<td>";
        html +=             "<label>PASS</label>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input type='password' name='pass' length=32>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input type='submit'>";
        html +=         "</td>";
        html +=     "</tr>";
        html += "</form>";
        if (httpd_chunk_send(arg, 2, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 2:
    {
        // OTA
        string ota;
        int fd = fs_open("ota", "r");
        if (fd >= 0)
        {
            ota = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        html += "<form method='get' action='ota'>";
        html +=     "<tr>";
        html +=         "<td>";
        html +=             "<label>OTA</label>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input name='ota' length=32 value='" + ota + "'>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input type='submit'>";
        html +=         "</td>";
        html +=     "</tr>";
        html += "</form>";
        if (httpd_chunk_send(arg, 3, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 3:
    {
        // MQTT
        string mqtt;
        string mqttPort;
        int fd = fs_open("mqtt", "r");
        if (fd >= 0)
        {
            mqtt = fs_gets(number, 128, fd);
            mqttPort = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        if (mqttPort.empty())
        {
            mqttPort = "1883";
        }
        html += "<form method='get' action='mqtt'>";
        html +=     "<tr>";
        html +=         "<td>";
        html +=             "<label>MQTT</label>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input name='mqtt' length=32 value='" + mqtt + "'>";
        html +=         "</td>";
        html +=     "</tr>";
        html +=     "<tr>";
        html +=         "<td>";
        html +=             "<label>PORT</label>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input name='port' length=32 value='" + mqttPort + "'>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input type='submit'>";
        html +=         "</td>";
        html +=     "</tr>";
        html += "</form>";
        if (httpd_chunk_send(arg, 4, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 4:
    {
        // NTP
        string ntp;
        string ntpZone;
        int fd = fs_open("ntp", "r");
        if (fd >= 0)
        {
            ntp = fs_gets(number, 128, fd);
            ntpZone = fs_gets(number, 128, fd);
            fs_close(fd);
        }
        if (ntp.empty())
        {
            ntp = "pool.ntp.org";
        }
        if (ntpZone.empty())
        {
            ntpZone = "8";
        }
        html += "<form method='get' action='ntp'>";
        html +=     "<tr>";
        html +=         "<td>";
        html +=             "<label>NTP</label>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input name='name' length=32 value='" + ntp + "'>";
        html +=         "</td>";
        html +=     "</tr>";
        html +=     "<tr>";
        html +=         "<td>";
        html +=             "<label>ZONE</label>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input name='zone' length=32 value='" + ntpZone + "'>";
        html +=         "</td>";
        html +=         "<td>";
        html +=             "<input type='submit'>";
        html +=         "</td>";
        html +=     "</tr>";
        html += "</form>";
        if (httpd_chunk_send(arg, 5, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    case 5:
    {
        html += "</table>";
        html += sntp_get_real_time(sntp_get_current_timestamp());

        // Reset
        html += "<form method='get' action='reset'>";
        html +=     "<button type='submit'>Reset</button>";
        html += "</form>";

        // Tail
        html += "</body>";
        html += "</html>";
        if (httpd_chunk_send(arg, 6, html.data(), html.length()) == false)
            return true;
        html.clear();
    }
    default:
        break;
    }

    return false;
}

bool web_ssid(void* arg, const char* url, int line)
{
    httpd_redirect(arg, "/");

    string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        string& text = *(string*)context;
        if (os_strcmp(key, "ssid") == 0 ||
            os_strcmp(key, "pass") == 0)
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

    string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        string& text = *(string*)context;
        if (os_strcmp(key, "ip") == 0 ||
            os_strcmp(key, "gateway") == 0 ||
            os_strcmp(key, "subnet") == 0 ||
            os_strcmp(key, "dns") == 0)
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

    string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        string& text = *(string*)context;
        if (os_strcmp(key, "ota") == 0)
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

    string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        string& text = *(string*)context;
        if (os_strcmp(key, "mqtt") == 0 ||
            os_strcmp(key, "port") == 0)
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

    string text;
    httpd_parameter_parse(url, [](void* context, const char* key, const char* value)
    {
        string& text = *(string*)context;
        if (os_strcmp(key, "name") == 0 ||
            os_strcmp(key, "zone") == 0)
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

bool web_rtc(void* arg, const char* url, int stage)
{
    if (stage < 192)
    {
        uint32_t data = 0;
        system_rtc_mem_read(stage, &data, sizeof(uint32_t));
        httpd_chunk_send(arg, stage + 1, number, os_sprintf(number, "%-3d:%08X\n", stage - 64, data));
        return true;
    }

    return false;
}
