#include "nvs.h"

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

#include "nvs_flash.h"

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
