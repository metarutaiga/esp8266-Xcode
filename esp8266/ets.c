#include "esp8266.h"

typedef struct ETSTabTask
{
    os_task_t task;
    os_event_t* event;
    uint8 size;
    uint8 reserved;
    uint8 offset;
    uint8 count;
    uint32 priority;
} tab_task;

extern uint8 ets_bit_task_priority;
extern uint32 ets_bit_count_task;
extern tab_task ets_tab_task[32];

void ets_run_once()
{
    uint8 priority = 32 - __builtin_clz(ets_bit_count_task);
    if (priority && ets_bit_count_task)
    {
        tab_task* tab = &ets_tab_task[priority - 1];
        os_event_t* event = &tab->event[tab->offset++];
        if (tab->size == tab->offset)
        {
            tab->offset = 0;
        }
        if (--tab->count == 0)
        {
            ets_bit_count_task &= ~tab->priority;
        }
        ets_bit_task_priority = priority;
        tab->task(event);
        ets_bit_task_priority = 0;
    }
}
