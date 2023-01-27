// Copyright 2018-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include "sdkconfig.h"

#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "esp_log.h"
#include "esp_phy_init.h"
#include "esp_heap_caps_init.h"
#include "esp_task_wdt.h"
#include "esp_private/wifi.h"
#include "esp_private/esp_system_internal.h"
#include "esp8266/eagle_soc.h"
#include "esp8266/pin_mux_register.h"
#include "esp8266/spi_register.h"

#include "FreeRTOS.h"
#include "task.h"
#include "esp_task.h"

#include "esp_newlib.h"

extern esp_err_t esp_pthread_init(void);
extern void chip_boot(void);
extern int base_gpio_init(void);

static void user_init_entry(void *param)
{
    void (**func)(void);

    extern void (*__init_array_start)(void);
    extern void (*__init_array_end)(void);

    extern void app_main(void);
    extern uint32_t esp_get_time(void);

    /* initialize C++ construture function */
    for (func = &__init_array_start; func < &__init_array_end; func++)
        func[0]();

    esp_phy_init_clk();
    assert(base_gpio_init() == 0);

    if (esp_reset_reason_early() != ESP_RST_FAST_SW) {
        assert(esp_mac_init() == ESP_OK);
    }

#if CONFIG_RESET_REASON
    esp_reset_reason_init();
#endif

#ifdef CONFIG_ESP_TASK_WDT
    esp_task_wdt_init();
#endif

    assert(esp_pthread_init() == 0);

#ifdef CONFIG_BOOTLOADER_FAST_BOOT
    REG_CLR_BIT(DPORT_CTL_REG, DPORT_CTL_DOUBLE_CLK);
#endif

#ifdef CONFIG_ESP8266_DEFAULT_CPU_FREQ_160
    esp_set_cpu_freq(ESP_CPU_FREQ_160M);
#endif

    app_main();

    vTaskDelete(NULL);
}

__attribute__((noinline))
static void call_start_cpu()
{
    int *p;

    extern int _bss_start, _bss_end;
    extern int _iram_bss_start, _iram_bss_end;

#ifdef CONFIG_BOOTLOADER_FAST_BOOT
    REG_SET_BIT(DPORT_CTL_REG, DPORT_CTL_DOUBLE_CLK);
#endif

    /*
     * When finish copying IRAM program, the exception vect must be initialized.
     * And then user can load/store data which is not aligned by 4-byte.
     */
    __asm__ __volatile__(
        "movi       a0, 0x40100000\n"
        "wsr        a0, vecbase\n"
        : : :"memory");

#ifndef CONFIG_BOOTLOADER_INIT_SPI_FLASH
    chip_boot();
#elif 1
    void esp_spi_flash_init(uint32_t spi_speed, uint32_t spi_mode);
    esp_spi_flash_init(0, 3);
#else
    SET_PERI_REG_MASK(PERIPHS_SPI_FLASH_USRREG, BIT5);
    CLEAR_PERI_REG_MASK(PERIPHS_SPI_FLASH_CTRL, SPI_FLASH_CLK_EQU_SYSCLK);
    CLEAR_PERI_REG_MASK(PERIPHS_IO_MUX_CONF_U, SPI0_CLK_EQU_SYSCLK);
    SET_PERI_REG_BITS(PERIPHS_SPI_FLASH_CTRL, 0xfff, 0x101, 0);
#endif

    /* clear bss data */
    for (p = &_bss_start; p < &_bss_end; p++)
        *p = 0;

    /* clear iram_bss data */
    for (p = &_iram_bss_start; p < &_iram_bss_end; p++)
        *p = 0;

    __asm__ __volatile__(
        "rsil       a2, 2\n"
        "movi       a1, _chip_interrupt_tmp\n"
        : : :"memory");

    heap_caps_init();

#ifdef CONFIG_INIT_OS_BEFORE_START
    extern int __esp_os_init(void);
    assert(__esp_os_init() == 0);
#endif

    assert(esp_newlib_init() == 0);

    assert(xTaskCreate(user_init_entry, "uiT", ESP_TASK_MAIN_STACK, NULL, ESP_TASK_MAIN_PRIO, NULL) == pdPASS);

    vTaskStartScheduler();
}

void IRAM_ATTR call_start_cpu_compatible()
{
    void Cache_Read_Disable();
    void Cache_Read_Enable(uint8_t sub_region, uint8_t region, uint8_t cache_size);
    uint32_t sub_region = GET_PERI_REG_BITS(CACHE_FLASH_CTRL_REG, 25, 24);
    uint32_t region = GET_PERI_REG_BITS(CACHE_FLASH_CTRL_REG, 18, 16);
    Cache_Read_Disable();
    Cache_Read_Enable(sub_region, region, 0);

    return call_start_cpu();
}
