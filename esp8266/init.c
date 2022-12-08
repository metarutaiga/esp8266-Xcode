#include "esp8266.h"

static void init_down(void)
{
    system_set_os_print(1);
    setup();
    system_os_post(USER_TASK_PRIO_1, 0, 0);
}

static void loop_task(os_event_t* event)
{
    loop();
    system_os_post(USER_TASK_PRIO_1, 0, 0);
}

void user_pre_init(void)
{
    static const partition_item_t at_partition_table[] PROGMEM =
    {
        { SYSTEM_PARTITION_RF_CAL,           0x3fb000,  0x1000 },
        { SYSTEM_PARTITION_PHY_DATA,         0x3fc000,  0x1000 },
        { SYSTEM_PARTITION_SYSTEM_PARAMETER, 0x3fd000,  0x3000 },
    };
    system_partition_table_regist(at_partition_table, sizeof(at_partition_table) / sizeof(at_partition_table[0]), system_get_flash_size_map());
}

void user_init(void)
{
    static os_event_t loop_event;
    system_os_task(loop_task, USER_TASK_PRIO_1, &loop_event, 1);
    system_init_done_cb(init_down);
}

static void delay_end(void* timer_arg)
{
    system_os_post(USER_TASK_PRIO_1, 0, 0);
}

void delay(unsigned int ms)
{
    static os_timer_t delay_timer;
    if (ms)
    {
        os_timer_setfn(&delay_timer, delay_end, 0);
        os_timer_arm(&delay_timer, ms, 1);
    }
    system_os_post(USER_TASK_PRIO_1, 0, 0);
    if (ms)
    {
        os_timer_disarm(&delay_timer);
    }
}
