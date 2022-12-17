#include "esp8266.h"
#include <string>
#include "http.h"

bool web_system(void *arg, int line)
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
#if 0
        if (File file = LittleFS.open(String(F("ssid")).c_str(), String(F("r")).c_str())) {
            ssid = file.readStringUntil('\n'); ssid.trim();
        }
#endif
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
#if 0
        if (File file = LittleFS.open(String(F("ip")).c_str(), String(F("r")).c_str())) {
            ip = file.readStringUntil('\n'); ssid.trim();
            gateway = file.readStringUntil('\n'); ssid.trim();
            subnet = file.readStringUntil('\n'); ssid.trim();
            dns = file.readStringUntil('\n'); ssid.trim();
        }
#endif
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
#if 0
        if (File file = LittleFS.open(String(F("ota")).c_str(), String(F("r")).c_str())) {
            ota = file.readStringUntil('\n'); ota.trim();
        }
#endif
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
#if 0
        if (File file = LittleFS.open(String(F("mqtt")).c_str(), String(F("r")).c_str())) {
            mqtt = file.readStringUntil('\n'); mqtt.trim();
            mqttPort = file.readStringUntil('\n'); mqttPort.trim();
        }
#endif
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
#if 0
        if (File file = LittleFS.open(String(F("ntp")).c_str(), String(F("r")).c_str())) {
            ntp = file.readStringUntil('\n'); ntp.trim();
            ntpZone = file.readStringUntil('\n'); ntpZone.trim();
        }
#endif
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
