/* Automatically generated file; DO NOT EDIT */
/* Espressif IoT Development Framework Linker Script */
/* Generated from: /Users/metora/Documents/esp8266-Xcode/ESP8266_RTOS_SDK/components/esp8266/ld/esp8266.project.ld.in */

/*  Default entry point:  */
ENTRY(call_start_cpu);

SECTIONS
{
  /* RTC data section holds RTC wake data/rodata
     marked with RTC_DATA_ATTR, RTC_RODATA_ATTR attributes.
  */
  .rtc.data :
  {
    _rtc_data_start = ABSOLUTE(.);

    *( .rtc.data  .rtc.data.*  .rtc.rodata  .rtc.rodata.*)

    _rtc_data_end = ABSOLUTE(.);
  } > rtc_data_seg

  /* RTC bss */
  .rtc.bss (NOLOAD) :
  {
    _rtc_bss_start = ABSOLUTE(.);

    *( .rtc.bss)

    _rtc_bss_end = ABSOLUTE(.);
  } > rtc_data_seg

  /* This section holds data that should not be initialized at power up
     and will be retained during deep sleep.
     User data marked with RTC_NOINIT_ATTR will be placed
     into this section. See the file "esp_attr.h" for more information.
  */
  .rtc_noinit (NOLOAD):
  {
    . = ALIGN(4);
    _rtc_noinit_start = ABSOLUTE(.);
    *(.rtc_noinit .rtc_noinit.*)
    . = ALIGN(4) ;
    _rtc_noinit_end = ABSOLUTE(.);
  } > rtc_data_seg

  ASSERT(((_rtc_noinit_end - ORIGIN(rtc_data_seg)) <= LENGTH(rtc_data_seg)),
        "RTC segment data does not fit.")

  /* Send .iram0 code to iram */
  .iram0.vectors :
  {
    _iram_start = ABSOLUTE(.);
    /* Vectors go to IRAM */
    _init_start = ABSOLUTE(.);
    KEEP(*(.SystemInfoVector.text));
    . = 0x10;
    KEEP(*(.DebugExceptionVector.text));
    . = 0x20;
    KEEP(*(.NMIExceptionVector.text));
    . = 0x30;
    KEEP(*(.KernelExceptionVector.text));
    . = 0x50;
    KEEP(*(.UserExceptionVector.text));
    . = 0x70;
    KEEP(*(.DoubleExceptionVector.text));

    *(.*Vector.literal)

    *(.UserEnter.literal);
    *(.UserEnter.text);
    . = ALIGN (16);
    *(.entry.text)
    *(.init.literal)
    *(.init)
    _init_end = ABSOLUTE(.);
  } > iram0_0_seg

  .iram0.text :
  {
    /* Code marked as runnning out of IRAM */
    _iram_text_start = ABSOLUTE(.);

    *libesp8266.a:(.data.WIFI_EVENT .data.g_esp_ticks_per_us)
    *libnet80211.a:ieee80211_nvs.o(.data.g_wifi_nvs)
    *libnet80211.a:ieee80211_api.o(.data.g_wifi_event_mask)
    *libnet80211.a:ieee80211_action.o(.data.*)
    *libphy.a:(.data.g_phyFuns .data.chip_v6_phyops .data.chip_version)
    *libpp.a:(.data.cur_timeout .data.cur_early .data.pTxRx .data.wDev_sniffer_max_len_filter .data.wDev_sniffer_payload_data_len .data.BcnInterval .data.g_sniffer_filter .data.sniffer_enable .data.WDEV_INTEREST_EVENT)
    *liblog.a:log.o(.data.s_putchar_func)
    *libc_nano.a:lib_a-locale.o(.data.__global_locale_ptr)
    *libtcpip_adapter.a:tcpip_adapter_lwip.o(.data.IP_EVENT)
    *liblwip.a:(.data.dhcps_lease_time .data.s_dhcps_mask .data.tcp_next_iss.iss)

    *( .iram1  .iram1.* EXCLUDE_FILE(*libpp.a *libphy.a *libgcc.a:unwind-dw2.* *libgcc.a:unwind-dw2-fde.* *libspi_flash.a:spi_flash_raw.*) .literal EXCLUDE_FILE(*libpp.a *libphy.a *libgcc.a:unwind-dw2.* *libgcc.a:unwind-dw2-fde.* *libspi_flash.a:spi_flash_raw.*) .text)
    *libpp.a:( .literal  .literal.*  .text  .text.*)
    *libphy.a:( .literal  .literal.*  .text  .text.*)
    *libspi_flash.a:spi_flash_raw.*( .literal  .literal.*  .text  .text.*)
    *libfreertos.a:xtensa_vectors.o( .data)

    _iram_text_end = ABSOLUTE(.);
  } > iram0_0_seg

  .iram0.bss :
  {
    . = ALIGN (4);
    /* Code marked as runnning out of IRAM */
    _iram_bss_start = ABSOLUTE(.);

    *libcore.a:( .bss  .bss.*  COMMON)
    *libfreertos.a:impure.*( .bss  .bss.*  COMMON)
    *libfreertos.a:tasks.*( .bss  .bss.*  COMMON)
    *libfreertos.a:list.*( .bss  .bss.*  COMMON)
    *libfreertos.a:timers.*( .bss  .bss.*  COMMON)
    *libfreertos.a:queue.*( .bss  .bss.*  COMMON)
    *libfreertos.a:stream_buffer.*( .bss  .bss.*  COMMON)
    *libfreertos.a:freertos_hooks.*( .bss  .bss.*  COMMON)
    *libfreertos.a:event_groups.*( .bss  .bss.*  COMMON)
    *liblwip.a:( .bss  .bss.*  COMMON)

    *libc_nano.a:(.bss.environ .bss.__sinit_recursive_mutex .bss.__sfp_recursive_mutex .bss._global_impure_ptr)
    *libesp8266.a:(.bss.p_uart_obj .bss.rx_gain_dc_table .bss.s_nmi_wd_state .bss.s_reset_reason .bss.s_sleep_wakup_triggers .bss.s_sleep_mode)
    *libesp_event.a:default_event_loop.o(.bss.s_default_loop)
    *libfreertos.a:(.bss.cpu_sr .bss.g_esp_boot_ccount .bss.g_esp_os_ticks .bss.g_esp_os_us .bss.g_esp_os_cpu_clk .bss.s_isr .bss.uxCriticalNesting .bss._xt_tick_divisor)
    *libheap.a:(.bss.g_heap_region)
    *liblog.a:log.o(.bss.s_lock)
    *libnet80211.a:(.bss.probesize .bss.BcnIntvl .bss.BcnEb .bss.arg_slp .bss.cb_slp .bss.type_slp .bss.wifi_sta_rx_probe_req .bss.avs_rx_content .bss.get_key_cb .bss.avs_cb)
    *libnewlib.a:time.o(.bss.s_boot_time)
    *libphy.a:(.bss.check_result .bss.esp_phyops .bss.adc_rand_noise .bss.rxiq_compute_num)
    *libpp.a:(.bss.app_optimize_and_debug_mac .bss.tsf_diff .bss.s_pp_queue_wptr .bss.s_pp_queue_rptr .bss.xphyQueue .bss.g_free_rxblock_eb_cnt .bss.peb_space .bss.prxdesc .bss.ptxdesc .bss.s_lmac_check_rx_hang .bss.updateCounter .bss.wDev_sniffer_min_len_filter .bss.BcnSendTick .bss.rxbufbit .bss.frontdata .bss.extradata .bss.hwtxrxbuf)
    *libpthread.a:(.bss.s_pthread_cfg_key .bss.s_threads_mux .bss.s_keys)
    *libtcpip_adapter.a:tcpip_adapter_lwip.o(.bss.dhcps_status .bss.esp_ip_lost_timer)
    *libvfs.a:(.bss.s_fd_table_lock .bss.s_vfs_count .bss.s_registered_select_num .bss.s_registered_selects .bss.s_vfs)
    *libwpa_supplicant.a:wpa_auth.o(.bss.s_sm_table .bss.s_sm_valid_bitmap)
    *libfreertos.a:xtensa_vectors.o(.bss)

    . = ALIGN (4);
    _iram_bss_end = ABSOLUTE(.);
    _iram_end = ABSOLUTE(.);
  } > iram0_0_seg

  ASSERT(((_iram_end - ORIGIN(iram0_0_seg)) <= LENGTH(iram0_0_seg)),
          "IRAM0 segment data does not fit.")

  .dram0.data :
  {
    _data_start = ABSOLUTE(.);
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    *(.dram0 .dram0.*)

    *( .data  .data.*  .dram1  .dram1.*)
    *liblog.a:( .rodata  .rodata.*)
    *libspi_flash.a:spi_flash_raw.*( .rodata  .rodata.*)

    _data_end = ABSOLUTE(.);
    . = ALIGN(4);
  } > dram0_0_seg

  /*This section holds data that should not be initialized at power up.
    The section located in Internal SRAM memory region. The macro _NOINIT
    can be used as attribute to place data into this section.
    See the esp_attr.h file for more information.
  */
  .noinit (NOLOAD):
  {
    . = ALIGN(4);
    _noinit_start = ABSOLUTE(.);
    *(.noinit .noinit.*)
    . = ALIGN(4) ;
    _noinit_end = ABSOLUTE(.);
  } > dram0_0_seg

  /* Shared RAM */
  .dram0.bss (NOLOAD) :
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);

    *(EXCLUDE_FILE(*libcore.a *libfreertos.a:event_groups.* *libfreertos.a:freertos_hooks.* *libfreertos.a:stream_buffer.* *libfreertos.a:queue.* *libfreertos.a:timers.* *libfreertos.a:list.* *libfreertos.a:tasks.* *libfreertos.a:impure.* *liblwip.a) .bss EXCLUDE_FILE(*libcore.a *libfreertos.a:event_groups.* *libfreertos.a:freertos_hooks.* *libfreertos.a:stream_buffer.* *libfreertos.a:queue.* *libfreertos.a:timers.* *libfreertos.a:list.* *libfreertos.a:tasks.* *libfreertos.a:impure.* *liblwip.a) .bss.* EXCLUDE_FILE(*libcore.a *libfreertos.a:event_groups.* *libfreertos.a:freertos_hooks.* *libfreertos.a:stream_buffer.* *libfreertos.a:queue.* *libfreertos.a:timers.* *libfreertos.a:list.* *libfreertos.a:tasks.* *libfreertos.a:impure.* *liblwip.a) COMMON)

    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    *(.share.mem)
    *(.gnu.linkonce.b.*)

    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
  } > dram0_0_seg

  ASSERT(((_bss_end - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
          "DRAM segment data does not fit.")

  .flash.text :
  {
    _stext = .;
    _text_start = ABSOLUTE(.);

    *(.rodata.__func__* .rodata.__FUNCTION__* .rodata.*str*)
    *libcore.a:user_interface.o(.rodata2.2)
    *libnet80211.a:(.rodata .rodata.g_wifi_nvs_cfg .rodata2.*)
    *libesp_event.a:(.rodata)
    *libphy.a:phy_chip_v6.o(.rodata)
    *libpp.a:(.rodata .rodata2.*)
    *(.rodata._* .rodata..L* .rodata.err_to_errno_table .rodata.phy_init_data)
    *(.rodata.K .rodata.Td0 .rodata.Td4s .rodata.Te0 .rodata.crc*)
    *libwpa_supplicant.a:md5-internal.o*(.literal.MD5*)
    *libwpa_supplicant.a:sha1-internal.o*(.literal.SHA1*)
    *(.rodata.[a-z]*)
    *(.rodata .rodata.* .rodata2 .rodata2.*)

    *(EXCLUDE_FILE(*libpp.a *libphy.a *libgcc.a:unwind-dw2.* *libgcc.a:unwind-dw2-fde.* *libspi_flash.a:spi_flash_raw.*) .literal EXCLUDE_FILE(*libpp.a *libphy.a *libgcc.a:unwind-dw2.* *libgcc.a:unwind-dw2-fde.* *libspi_flash.a:spi_flash_raw.*) .literal.* EXCLUDE_FILE(*libpp.a *libphy.a *libgcc.a:unwind-dw2.* *libgcc.a:unwind-dw2-fde.* *libspi_flash.a:spi_flash_raw.*) .text EXCLUDE_FILE(*libpp.a *libphy.a *libgcc.a:unwind-dw2.* *libgcc.a:unwind-dw2-fde.* *libspi_flash.a:spi_flash_raw.*) .text.*  .wifi0iram  .wifi0iram.*)
    *libgcc.a:unwind-dw2-fde.*( .literal  .literal.*  .text  .text.*)
    *libgcc.a:unwind-dw2.*( .literal  .literal.*  .text  .text.*)

    /* For ESP8266 library function */
    *(.irom0.literal .irom0.text)
    *(.irom.literal .irom.text .irom.text.literal)
    *(.text2 .text2.* .literal2 .literal2.*)

    *(.stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.irom0.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.fini.literal)
    *(.fini)
    *(.gnu.version)
    _text_end = ABSOLUTE(.);
    _etext = .;

    /* Similar to _iram_start, this symbol goes here so it is
       resolved by addr2line in preference to the first symbol in
       the flash.text segment.
    */
    _flash_cache_start = ABSOLUTE(0);
  } >iram0_2_seg

  .flash.rodata ALIGN(4) :
  {
    _rodata_start = ABSOLUTE(.);

   /**
      Insert 8 bytes data to make realy rodata section's link address offset to be 0x8,
      esptool will remove these data and add real segment header
    */
    . = 0x8;

    *(.rodata_desc .rodata_desc.*)               /* Should be the first.  App version info.        DO NOT PUT ANYTHING BEFORE IT! */
    *(.rodata_custom_desc .rodata_custom_desc.*) /* Should be the second. Custom app version info. DO NOT PUT ANYTHING BEFORE IT! */

    *(.rodata2 .rodata2.*)                       /* For ESP8266 library function */

    *(EXCLUDE_FILE(*liblog.a *libspi_flash.a:spi_flash_raw.*) .rodata EXCLUDE_FILE(*liblog.a *libspi_flash.a:spi_flash_raw.*) .rodata.*)

    *(.irom1.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE_ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    . = (. + 3) & ~ 3;
    __eh_frame = ABSOLUTE(.);
    KEEP(*(.eh_frame))
    . = (. + 7) & ~ 3;
    /*  C++ constructor and destructor tables

        Make a point of not including anything from crtbegin.o or crtend.o, as IDF doesn't use toolchain crt
      */
    __init_array_start = ABSOLUTE(.);
    KEEP (*(EXCLUDE_FILE (*crtend.* *crtbegin.*) .ctors .ctors.*))
    __init_array_end = ABSOLUTE(.);
    KEEP (*crtbegin.*(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.*) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS_ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    /* Addresses of memory regions reserved via
       SOC_RESERVE_MEMORY_REGION() */
    soc_reserved_memory_region_start = ABSOLUTE(.);
    KEEP (*(.reserved_memory_address))
    soc_reserved_memory_region_end = ABSOLUTE(.);
    _rodata_end = ABSOLUTE(.);
    /* Literals are also RO data. */
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
    . = ALIGN(4);
    _thread_local_start = ABSOLUTE(.);
    *(.tdata)
    *(.tdata.*)
    *(.tbss)
    *(.tbss.*)
    _thread_local_end = ABSOLUTE(.);
    . = ALIGN(4);
  } >iram0_2_seg
}
