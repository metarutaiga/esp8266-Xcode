#include "eagle.h"
#include <nvs.h>

#define TAG __FILE_NAME__

esp_err_t nvs_open(const char* name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle) { return ESP_FAIL; }
esp_err_t nvs_open_from_partition(const char *part_name, const char* name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle) { return ESP_FAIL; }
esp_err_t nvs_set_i8(nvs_handle_t handle, const char* key, int8_t value) { return ESP_FAIL; }
esp_err_t nvs_set_u8(nvs_handle_t handle, const char* key, uint8_t value) { return ESP_FAIL; }
esp_err_t nvs_set_i16(nvs_handle_t handle, const char* key, int16_t value) { return ESP_FAIL; }
esp_err_t nvs_set_u16(nvs_handle_t handle, const char* key, uint16_t value) { return ESP_FAIL; }
esp_err_t nvs_set_i32(nvs_handle_t handle, const char* key, int32_t value) { return ESP_FAIL; }
esp_err_t nvs_set_u32(nvs_handle_t handle, const char* key, uint32_t value) { return ESP_FAIL; }
esp_err_t nvs_set_i64(nvs_handle_t handle, const char* key, int64_t value) { return ESP_FAIL; }
esp_err_t nvs_set_u64(nvs_handle_t handle, const char* key, uint64_t value) { return ESP_FAIL; }
esp_err_t nvs_set_str(nvs_handle_t handle, const char* key, const char* value) { return ESP_FAIL; }
esp_err_t nvs_set_blob(nvs_handle_t handle, const char* key, const void* value, size_t length) { return ESP_FAIL; }
esp_err_t nvs_get_i8(nvs_handle_t handle, const char* key, int8_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_u8(nvs_handle_t handle, const char* key, uint8_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_i16(nvs_handle_t handle, const char* key, int16_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_u16(nvs_handle_t handle, const char* key, uint16_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_i32(nvs_handle_t handle, const char* key, int32_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_u32(nvs_handle_t handle, const char* key, uint32_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_i64(nvs_handle_t handle, const char* key, int64_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_u64(nvs_handle_t handle, const char* key, uint64_t* out_value) { return ESP_FAIL; }
esp_err_t nvs_get_str(nvs_handle_t handle, const char* key, char* out_value, size_t* length) { return ESP_FAIL; }
esp_err_t nvs_get_blob(nvs_handle_t handle, const char* key, void* out_value, size_t* length) { return ESP_FAIL; }
esp_err_t nvs_erase_key(nvs_handle_t handle, const char* key) { return ESP_FAIL; }
esp_err_t nvs_erase_all(nvs_handle_t handle) { return ESP_FAIL; }
esp_err_t nvs_commit(nvs_handle_t handle) { return ESP_FAIL; }
void nvs_close(nvs_handle_t handle) {}
esp_err_t nvs_get_stats(const char *part_name, nvs_stats_t *nvs_stats) { return ESP_FAIL; }
esp_err_t nvs_get_used_entry_count(nvs_handle_t handle, size_t* used_entries) { return ESP_FAIL; }
nvs_iterator_t nvs_entry_find(const char *part_name, const char *namespace_name, nvs_type_t type) { return NULL; }
nvs_iterator_t nvs_entry_next(nvs_iterator_t iterator) { return NULL; }
void nvs_entry_info(nvs_iterator_t iterator, nvs_entry_info_t *out_info) {}
void nvs_release_iterator(nvs_iterator_t iterator) {}

#include <nvs_flash.h>

esp_err_t nvs_flash_init(void) { return ESP_FAIL; }
esp_err_t nvs_flash_init_partition(const char *partition_label) { return ESP_FAIL; }
esp_err_t nvs_flash_init_partition_ptr(const esp_partition_t *partition) { return ESP_FAIL; }
esp_err_t nvs_flash_deinit(void) { return ESP_FAIL; }
esp_err_t nvs_flash_deinit_partition(const char* partition_label) { return ESP_FAIL; }
esp_err_t nvs_flash_erase(void) { return ESP_FAIL; }
esp_err_t nvs_flash_erase_partition(const char *part_name) { return ESP_FAIL; }
esp_err_t nvs_flash_erase_partition_ptr(const esp_partition_t *partition) { return ESP_FAIL; }
esp_err_t nvs_flash_secure_init(nvs_sec_cfg_t* cfg) { return ESP_FAIL; }
esp_err_t nvs_flash_secure_init_partition(const char *partition_label, nvs_sec_cfg_t* cfg) { return ESP_FAIL; }
esp_err_t nvs_flash_generate_keys(const esp_partition_t* partition, nvs_sec_cfg_t* cfg) { return ESP_FAIL; }
esp_err_t nvs_flash_read_security_cfg(const esp_partition_t* partition, nvs_sec_cfg_t* cfg) { return ESP_FAIL; }

#define NVS_WIFI_CFG \
    _( 0, 0, 0,    1,   "opmode"), \
    _( 1, 7, 4,    36,  "sta.ssid"), \
    _( 2, 7, 40,   6,   "sta.mac"), \
    _( 3, 0, 46,   1,   "sta.authmode"), \
    _( 4, 6, 47,   65,  "sta.pswd"), \
    _( 5, 7, 112,  32,  "sta.pmk"), \
    _( 6, 0, 144,  1,   "sta.chan"), \
    _( 7, 0, 145,  1,   "auto.conn"), \
    _( 8, 0, 146,  1,   "bssid.set"), \
    _( 9, 7, 147,  6,   "sta.bssid"), \
    _(10, 2, 154,  2,   "sta.lis_intval"), \
    _(11, 0, 156,  1,   "sta.phym"), \
    _(12, 0, 157,  1,   "sta.phybw"), \
    _(13, 7, 158,  2,   "sta.apsw"), \
    _(14, 7, 160,  700, "sta.apinfo"), \
    _(15, 0, 860,  1,   "sta.scan_method"), \
    _(16, 0, 861,  1,   "sta.sort_method"), \
    _(17, 1, 862,  1,   "sta.minrssi"), \
    _(18, 0, 863,  1,   "sta.minauth"), \
    _(19, 7, 868,  36,  "ap.ssid"), \
    _(20, 7, 904,  6,   "ap.mac"), \
    _(21, 6, 910,  65,  "ap.passwd"), \
    _(22, 7, 975,  32,  "ap.pmk"), \
    _(23, 0, 1007, 1,   "ap.chan"), \
    _(24, 0, 1008, 1,   "ap.authmode"), \
    _(25, 0, 1009, 1,   "ap.hidden"), \
    _(26, 0, 1010, 1,   "ap.max.conn"), \
    _(27, 2, 1012, 2,   "bcn.interval"), \
    _(28, 0, 1014, 1,   "ap.phym"), \
    _(29, 0, 1015, 1,   "ap.phybw"), \
    _(30, 0, 1016, 1,   "ap.sndchan"), \
    _(31, 0, 1,    1,   "lorate"), \
    _(32, 0, 864,  1,   "sta.pmf_e"), \
    _(33, 0, 865,  1,   "sta.pmf_r"), \
    _(34, 0, 1017, 1,   "ap.pmf_e"), \
    _(35, 0, 1018, 1,   "ap.pmf_r"), \
    _(36, 7, 1020, 12,  "country"), \
    _(37, 0, 866,  1,   "sta.rm_e"), \
    _(38, 0, 867,  1,   "sta.btm_e"), \

static const uint8_t g_wifi_cfg_type[39] = {
#   define _(a, b, c, d, e) b
    NVS_WIFI_CFG
#   undef _
};

static const uint16_t g_wifi_cfg_address[39] = {
#   define _(a, b, c, d,e ) c
    NVS_WIFI_CFG
#   undef _
};

static const uint16_t g_wifi_cfg_size[39] = {
#   define _(a, b, c, d, e) d
    NVS_WIFI_CFG
#   undef _
};

static const char* const g_wifi_cfg_name[39] __attribute__((unused)) = {
#   define _(a, b, c, d, e) e
    NVS_WIFI_CFG
#   undef _
};

extern void* g_wifi_nvs;

void __wrap_wifi_nvs_init()
{
}

void __wrap_wifi_nvs_deinit()
{
}

void __wrap_wifi_nvs_set(uint32_t index, uint32_t value)
{
    if (index > 38)
        return;
    uint8_t* s_wifi_nvs = g_wifi_nvs;
    switch (g_wifi_cfg_type[index]) {
    case 0: // nvs_set_u8
    case 1: // nvs_set_i8
    case 2: // nvs_set_u16
        memcpy(s_wifi_nvs + g_wifi_cfg_address[index], &value, g_wifi_cfg_size[index]);
        break;
    case 3:
    case 4:
    case 5:
        break;
    case 6: // strncpy
        strncpy(s_wifi_nvs + g_wifi_cfg_address[index], (void*)value, g_wifi_cfg_size[index]);
        break;
    case 7: // memcpy
        memcpy(s_wifi_nvs + g_wifi_cfg_address[index], (void*)value, g_wifi_cfg_size[index]);
        break;
    }
}

void* __wrap_wifi_nvs_get()
{
    return g_wifi_nvs;
}

uint16_t __wrap_wifi_nvs_get_sta_listen_interval()
{
    uint16_t* s_wifi_nvs = g_wifi_nvs;
    return s_wifi_nvs[g_wifi_cfg_address[10] / sizeof(uint16_t)];
}

void __wrap_wifi_nvs_commit()
{
}
