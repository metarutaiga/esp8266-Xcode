#include "esp8266.h"

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

typedef void (*_xtos_handler)(struct exception_frame *ef, int cause);
extern _xtos_handler _xtos_exc_handler_table[];
extern _xtos_handler _xtos_c_handler_table[];

static _xtos_handler origin_exception;
static void dump_exception(struct exception_frame *ef, int cause)
{
    extern int divide;
    if ((ef->epc & 0xffff0000) == 0x40000000)
    {
        
    }
    else if ((ef->epc >= (uint32_t)&divide) && (ef->epc < (uint32_t)&divide + 0x800))
    {
        // sprintf
    }
    else if ((ef->epc >= (uint32_t)&strsep) && (ef->epc < (uint32_t)&strsep + 0x88))
    {
        // strsep / strtok
    }
    else
    {
        uint32_t excvaddr;
        __asm__ __volatile__ ("rsr.excvaddr %0;" : "=r"(excvaddr):: "memory");
        static int entered = 0;
        if (entered == 0)
        {
            entered = 1;
            os_printf("%08x %08x %08x\n", ef->epc, excvaddr, ef->ps);
            entered = 0;
        }
    }
    origin_exception(ef, cause);
}

void hook_exception(void)
{
    origin_exception = _xtos_c_handler_table[3];
    _xtos_c_handler_table[3] = dump_exception;
}

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
    static const partition_item_t at_partition_table[] =
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
    hook_exception();
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
