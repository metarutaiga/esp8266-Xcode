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

#include "esp_phy_init.h"
#include "esp_heap_caps_init.h"
#include "esp_task_wdt.h"
#include "esp_private/esp_system_internal.h"
#include "esp8266/pin_mux_register.h"
#include "esp8266/spi_register.h"
#include "rom/uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "esp_task.h"

#include "esp_newlib.h"

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

    extern int base_gpio_init(void);
    assert(base_gpio_init() == 0);

    if (esp_reset_reason_early() != ESP_RST_FAST_SW) {
        assert(esp_mac_init() == ESP_OK);
    }

#ifdef CONFIG_RESET_REASON
    esp_reset_reason_init();
#endif

#ifdef CONFIG_ESP_TASK_WDT
    esp_task_wdt_init();
#endif

    extern esp_err_t esp_pthread_init(void);
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
    extern void chip_boot(void);
    chip_boot();
#elif 0
    extern void esp_spi_flash_init(uint32_t spi_speed, uint32_t spi_mode);
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
    void(*SPIRead)(uint32_t addr, uint32_t* dst, size_t size) = (void(*)(uint32_t, uint32_t*, uint32_t))0x40004B1C;

    uint32_t bank = 0;
    SPIRead(0x3FF000, &bank, sizeof(bank));

    uint32_t boot_param = 0;
    SPIRead((bank & 0xFF) == 0 ? 0x3FD000 : 0x3FE000, &boot_param, sizeof(boot_param));

    void Cache_Read_Disable();
    void Cache_Read_Enable(uint8_t sub_region, uint8_t region, uint8_t cache_size);
    switch (boot_param & 0x03)
    {
    default:
    case 0:
        Cache_Read_Disable();
        Cache_Read_Enable(0, 0, 0);
        break;
    case 1:
        Cache_Read_Disable();
        Cache_Read_Enable(1, 0, 0);
        break;
    }

    return call_start_cpu();
}

void esp_now_deinit()
{
    
}

void esp_reset(esp_reset_reason_t hint)
{
#if 0
    void __esp_soc_restart();
    __esp_soc_restart();

    Cache_Read_Disable();
    CLEAR_PERI_REG_MASK(0x3FF00024, 0x67);
    
    __asm__ __volatile__(
        "movi       a0, 0x40000000\n"
        "wsr        a0, vecbase\n"
        : : : "memory");

    __asm__ __volatile__(
        "mov        a1, 0x40000000\n"
        : : : "memory"
    );

    rom_software_reboot();
#elif 0
    extern void esp_wifi_stop(void);
    extern void pm_goto_rf_on(void);
    extern void clockgate_watchdog(int on);

    esp_wifi_stop();

    uart_tx_wait_idle(1);
    uart_tx_wait_idle(0);

    vTaskDelay(40 / portTICK_RATE_MS);

    pm_goto_rf_on();
    clockgate_watchdog(0);
    REG_WRITE(0x3ff00018, 0xffff00ff);
    SET_PERI_REG_MASK(0x60000D48, BIT1);
    CLEAR_PERI_REG_MASK(0x60000D48, BIT1);

    uart_disable_swap_io();

    vPortEnterCritical();
    REG_WRITE(INT_ENA_WDEV, 0);
    _xt_isr_mask(UINT32_MAX);

    esp_reset_reason_set_hint(hint);

    const uint32_t sp = DRAM_BASE + DRAM_SIZE - 16;

    __asm__ __volatile__(
        "mov    a1, %0\n"
        : : "a"(sp) : "memory"
    );

    call_start_cpu_compatible();
#else
    uart_tx_wait_idle(0);
    uart_tx_wait_idle(1);

    esp_reset_reason_set_hint(hint);

    CLEAR_WDT_REG_MASK(WDT_CTL_ADDRESS, BIT0);
    WDT_REG_WRITE(WDT_OP_ADDRESS, 1);
    WDT_REG_WRITE(WDT_OP_ND_ADDRESS, 1);
    SET_PERI_REG_BITS(PERIPHS_WDT_BASEADDR + WDT_CTL_ADDRESS, WDT_CTL_RSTLEN_MASK, 7 << WDT_CTL_RSTLEN_LSB, 0);
    SET_PERI_REG_BITS(PERIPHS_WDT_BASEADDR + WDT_CTL_ADDRESS, WDT_CTL_RSPMOD_MASK, 0 << WDT_CTL_RSPMOD_LSB, 0);
    SET_PERI_REG_BITS(PERIPHS_WDT_BASEADDR + WDT_CTL_ADDRESS, WDT_CTL_EN_MASK, 1 << WDT_CTL_EN_LSB, 0);

    while (1);
#endif
}
