#include "esp8266.h"

#ifdef DEMO
struct exception_frame
{
    uint32_t epc;
    uint32_t ps;
    uint32_t sar;
    uint32_t unused;
    uint32_t a0;
    // note: no a1 here!
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t a8;
    uint32_t a9;
    uint32_t a10;
    uint32_t a11;
    uint32_t a12;
    uint32_t a13;
    uint32_t a14;
    uint32_t cause;
};

typedef void (*_xtos_handler)(struct exception_frame* ef, int cause);
extern _xtos_handler _xtos_exc_handler_table[];
extern _xtos_handler _xtos_c_handler_table[];

static _xtos_handler origin_exception IRAM_ATTR;
static void dump_exception(struct exception_frame* ef, int cause)
{
    extern int divide;
    if ((ef->epc & 0xffff0000) == 0x40000000)
    {
        
    }
    else if ((ef->epc >= (uint32_t)&divide) && (ef->epc < (uint32_t)&divide + 0x800))
    {
        // sprintf
    }
    else
    {
        uint32_t excvaddr;
        __asm__ __volatile__ ("rsr.excvaddr %0;" : "=r"(excvaddr):: "memory");
        os_printf("%08x %08x %08x\n", ef->epc, excvaddr, ef->ps);
    }
    origin_exception(ef, cause);
}

static void hook_exception(void)
{
    origin_exception = _xtos_c_handler_table[3];
    _xtos_c_handler_table[3] = dump_exception;
}
#endif

static void init_down(void)
{
    system_set_os_print(1);
    setup();
    system_os_post(USER_TASK_PRIO_1, 0, 0);
}

#ifdef LOOP
void loop() __attribute__((weak));
void loop() {}
static void loop_task(os_event_t* event)
{
    loop();
    system_os_post(USER_TASK_PRIO_1, 0, 0);
}
#endif

void user_pre_init(void)
{
    static const partition_item_t at_partition_table[] =
    {
        { SYSTEM_PARTITION_BOOTLOADER,       0x000000,  0x01000 },
        { SYSTEM_PARTITION_OTA_1,            0x001000,  0xff000 },
        { SYSTEM_PARTITION_OTA_2,            0x101000,  0xff000 },
        { SYSTEM_PARTITION_RF_CAL,           0x3fb000,  0x01000 },
        { SYSTEM_PARTITION_PHY_DATA,         0x3fc000,  0x01000 },
        { SYSTEM_PARTITION_SYSTEM_PARAMETER, 0x3fd000,  0x03000 },
    };
    system_partition_table_regist(at_partition_table, sizeof(at_partition_table) / sizeof(at_partition_table[0]), system_get_flash_size_map());
}

static uint32_t system_get_time_overflow_last IRAM_ATTR = 0;
static uint32_t system_get_time_overflow_count IRAM_ATTR = 0;

void system_get_time_overflow_callback(void* arg)
{
    uint32_t time = system_get_time();
    if (time < system_get_time_overflow_last)
    {
        system_get_time_overflow_count++;
    }
    system_get_time_overflow_last = time;
}

uint64_t IRAM_FLASH_ATTR system_get_time64()
{
    uint32_t low = system_get_time();
    uint32_t high = system_get_time_overflow_count + ((low < system_get_time_overflow_last) ? 1 : 0);
    return (uint64_t)high << 32 | low;
}

uint32_t IRAM_FLASH_ATTR system_get_time_ms()
{
    return (uint32_t)(system_get_time64() / 1000);
}

void user_init(void)
{
    system_set_os_print(1);
#ifdef LOOP
    static os_event_t loop_event IRAM_ATTR;
    system_os_task(loop_task, USER_TASK_PRIO_1, &loop_event, 1);
#endif
    static os_timer_t system_get_time_overflow_timer IRAM_ATTR;
    os_timer_setfn(&system_get_time_overflow_timer, system_get_time_overflow_callback, 0);
    os_timer_arm(&system_get_time_overflow_timer, 60000, true);
    system_init_done_cb(init_down);
#ifdef DEMO
    hook_exception();
#endif
}

static void delay_end(void* timer_arg)
{
    system_os_post(USER_TASK_PRIO_1, 0, 0);
}

void delay(unsigned int ms)
{
    static os_timer_t delay_timer IRAM_ATTR;
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
