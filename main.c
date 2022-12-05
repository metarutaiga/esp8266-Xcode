#include "c_types.h"
#include "osapi.h"
#include "user_interface.h"

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    static const partition_item_t at_partition_table[] PROGMEM =
    {
        { SYSTEM_PARTITION_RF_CAL,           0x3fb000,  0x1000 },
        { SYSTEM_PARTITION_PHY_DATA,         0x3fc000,  0x1000 },
        { SYSTEM_PARTITION_SYSTEM_PARAMETER, 0x3fd000,  0x3000 },
    };
    system_partition_table_regist(at_partition_table, sizeof(at_partition_table) / sizeof(at_partition_table[0]), system_get_flash_size_map());
}

void ICACHE_FLASH_ATTR user_init(void)
{
    system_set_os_print(1);
    os_printf_plus(PSTR("%s\n"), PSTR("Hello world!"));
    os_printf_plus(PSTR("RAM : %d\n"), system_get_free_heap_size());
}
