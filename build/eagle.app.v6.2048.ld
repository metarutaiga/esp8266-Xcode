/* This linker script generated from xt-genldscripts.tpp for LSP . */
/* Linker Script for ld -N */
MEMORY
{
  dport0_0_seg :                        org = 0x3FF00000, len = 0x10
  dram0_0_seg :                         org = 0x3FFE8000, len = 0x14000
  iram1_0_seg :                         org = 0x40100000, len = 0x8000
  irom0_0_seg :                         org = 0x40201010, len = 0xFEFF0
}

PHDRS
{
  dport0_0_phdr PT_LOAD;
  dram0_0_phdr PT_LOAD;
  dram0_0_bss_phdr PT_LOAD;
  iram1_0_phdr PT_LOAD;
  irom0_0_phdr PT_LOAD;
}


/*  Default entry point:  */
ENTRY(call_user_start)
EXTERN(_DebugExceptionVector)
EXTERN(_DoubleExceptionVector)
EXTERN(_KernelExceptionVector)
EXTERN(_NMIExceptionVector)
EXTERN(_UserExceptionVector)
PROVIDE(_memmap_vecbase_reset = 0x40000000);
/* Various memory-map dependent cache attribute settings: */
_memmap_cacheattr_wb_base = 0x00000110;
_memmap_cacheattr_wt_base = 0x00000110;
_memmap_cacheattr_bp_base = 0x00000220;
_memmap_cacheattr_unused_mask = 0xFFFFF00F;
_memmap_cacheattr_wb_trapnull = 0x2222211F;
_memmap_cacheattr_wba_trapnull = 0x2222211F;
_memmap_cacheattr_wbna_trapnull = 0x2222211F;
_memmap_cacheattr_wt_trapnull = 0x2222211F;
_memmap_cacheattr_bp_trapnull = 0x2222222F;
_memmap_cacheattr_wb_strict = 0xFFFFF11F;
_memmap_cacheattr_wt_strict = 0xFFFFF11F;
_memmap_cacheattr_bp_strict = 0xFFFFF22F;
_memmap_cacheattr_wb_allvalid = 0x22222112;
_memmap_cacheattr_wt_allvalid = 0x22222112;
_memmap_cacheattr_bp_allvalid = 0x22222222;
PROVIDE(_memmap_cacheattr_reset = _memmap_cacheattr_wb_trapnull);

SECTIONS
{

  .dport0.rodata : ALIGN(4)
  {
    _dport0_rodata_start = ABSOLUTE(.);
    *(.dport0.rodata)
    *(.dport.rodata)
    _dport0_rodata_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .dport0.literal : ALIGN(4)
  {
    _dport0_literal_start = ABSOLUTE(.);
    *(.dport0.literal)
    *(.dport.literal)
    _dport0_literal_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .dport0.data : ALIGN(4)
  {
    _dport0_data_start = ABSOLUTE(.);
    *(.dport0.data)
    *(.dport.data)
    _dport0_data_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .text : ALIGN(4)
  {
    _stext = .;
    _text_start = ABSOLUTE(.);
    *(.UserEnter.text)
    . = ALIGN(16);
    *(.DebugExceptionVector.text)
    . = ALIGN(16);
    *(.NMIExceptionVector.text)
    . = ALIGN(16);
    *(.KernelExceptionVector.text)
    LONG(0)
    LONG(0)
    LONG(0)
    LONG(0)
    . = ALIGN(16);
    *(.UserExceptionVector.text)
    LONG(0)
    LONG(0)
    LONG(0)
    LONG(0)
    . = ALIGN(16);
    *(.DoubleExceptionVector.text)
    LONG(0)
    LONG(0)
    LONG(0)
    LONG(0)
    . = ALIGN (16);
    *(.entry.text)
    *(.init.literal)
    *(.init)
    *libnet80211.a:ieee80211_input.o(.data)
    *homekit.o(.data..compoundliteral*)
    *libcJSON.a:cJSON.o(.data.global_hooks)
    *libesp-homekit.a:server.o(.data.homekit_http_parser_settings)
    *libhttp-parser.a:http_parser.o(.data.max_header_size)
    *liblwip.a:dhcpserver.o(.data.dhcps_lease_time)
    *liblwip.a:sntp.o(.data.sntp_update_delay)
    *liblwip.a:tcp.o(.data.tcp_next_iss.iss)
    *libmain.a:nmi-vector.o(.data)
    *libnet80211.a:ieee80211_action.o(.data)
    *libphy.a:phy_sleep.o(.data)
    *libnet80211.a:wl_chm.o(.text)
    *libc.a:(.literal .literal.*)
    *libgcc.a:(.literal .literal.*)
    *liblwip.a:(.literal .literal.*)
    *libmain.a:(.literal .literal.*)
    *libphy.a:(.literal .literal.*)
    *libpp.a:(.literal .literal.*)
    *libc.a:(.text .text.*)
    *libgcc.a:(.text .text.*)
    *liblwip.a:(.text .text.*)
    *libmain.a:(.text .text.*)
    *libphy.a:(.text .text.*)
    *libpp.a:(.text .text.*)
    *(*.iram0.text*)
    *(*.iram.text*)
    *libcrypto.a:random.o(.bss.entropy)
    *libcrypto.a:random.o(.bss.input_rotate)
    *libcrypto.a:random.o(.bss.pool_pos)
    *libcrypto.a:random.o(.bss.pool)
    *libcrypto.a:tls_internal.o(.bss.tls_ref_count)
    *liblwip.a:sntp.o(.bss.realtime_stamp)
    *liblwip.a:sntp.o(.bss.res_buf)
    *liblwip.a:sntp.o(.bss.sntp_pcb)
    *liblwip.a:sntp.o(.bss.sntp_retry_timeout)
    *liblwip.a:sntp.o(.bss.sntp_timer)
    *liblwip.a:udp.o(.bss.udp_pcbs)
    *liblwip.a:netif.o(.bss.netif_list)
    *liblwip.a:netif.o(.bss.netif_default)
    *liblwip.a:dhcpserver.o(.bss.netif_default)
    *liblwip.a:dhcpserver.o(.bss.pcb_dhcps)
    *liblwip.a:dhcpserver.o(.bss.broadcast_dhcps)
    *liblwip.a:dhcpserver.o(.bss.server_address*)
    *liblwip.a:dhcpserver.o(.bss.plist)
    *liblwip.a:dhcpserver.o(.bss.client_address)
    *liblwip.a:dns.o(.bss.dns_pcb)
    *liblwip.a:dns.o(.bss.dns_servers)
    *liblwip.a:dns.o(.bss.dns_payload)
    *liblwip.a:dhcp.o(.bss.vendor_class_buf)
    *liblwip.a:dhcp.o(.bss.vendor_class_len)
    *liblwip.a:dhcp.o(.bss.dhcp_rx_options_val)
    *liblwip.a:dhcp.o(.bss.dhcp_create_msg.xid)
    *liblwip.a:timers.o(.bss.timeouts_last_time)
    *liblwip.a:timers.o(.bss.next_timeout)
    *liblwip.a:tcp.o(.bss.tcp_listen_pcbs)
    *liblwip.a:tcp.o(.bss.tcp_bound_pcbs)
    *liblwip.a:tcp.o(.bss.tcp_active_pcbs)
    *liblwip.a:tcp.o(.bss.tcp_tw_pcbs)
    *liblwip.a:tcp.o(.bss.tcp_tmp_pcb)
    *liblwip.a:tcp.o(.bss.tcp_ticks)
    *liblwip.a:espconn.o(.bss.plink_active)
    *liblwip.a:espconn.o(.bss.pserver_list)
    *liblwip.a:espconn.o(.bss.espconn_port.randnum)
    *liblwip.a:igmp.o(.bss.allsystems)
    *liblwip.a:igmp.o(.bss.allrouters)
    *liblwip.a:igmp.o(.bss.igmp_group_list)
    *liblwip.a:ip.o(.bss.current_netif)
    *liblwip.a:ip.o(.bss.current_header)
    *liblwip.a:tcp_in.o(.bss.iphdr)
    *liblwip.a:tcp_in.o(.bss.tcphdr)
    *liblwip.a:tcp_in.o(.bss.seqno)
    *liblwip.a:tcp_in.o(.bss.ackno)
    *liblwip.a:tcp_in.o(.bss.recv_data)
    *liblwip.a:tcp_in.o(.bss.tcp_input_pcb)
    *liblwip.a:raw.o(.bss.raw_pcbs)
    *libmain.a:nmi-vector.o(.bss)
    *libmain.a:spi_flash.o(.bss)
    *libmain.a:user_exceptions.o(.bss)
    *libnet80211.a:ieee80211_ht.o(.bss)
    *libnet80211.a:ieee80211_output.o(.bss)
    *libphy.a:phy.o(.bss)
    *(.stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.fini.literal)
    *(.fini)
    *(.gnu.version)
    _text_end = ABSOLUTE(.);
    _etext = .;
  } >iram1_0_seg :iram1_0_phdr

  .lit4 : ALIGN(4)
  {
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
  } >iram1_0_seg :iram1_0_phdr

  .data : ALIGN(4)
  {
    _data_start = ABSOLUTE(.);
    *(SORT_BY_ALIGNMENT(.data*))
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    *(.rodata.CSWTCH*)
    *liblwip.a:etharp.o(.rodata.eth*)
    *libphy.a:phy_chip_v6.o(.rodata)
    *libphy.a:phy_chip_v6_ana.o(.rodata)
    *libphy.a:phy_chip_v6_cal.o(.rodata)
    *libpp.a:pp.o(.rodata)
    *libnet80211.a:wl_cnx.o(.rodata)
    _data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .irom0.text : ALIGN(4)
  {
    _irom0_text_start = ABSOLUTE(.);

    *(.sdk.version)
    *(*.rodata.__func__* *.rodata.str* *.pstr* EXCLUDE_FILE(ieee80211_phy.o ieee80211_crypto_tkip.o ieee80211_crypto_wep.o memp.o tcp.o) *.irom.text* *.rodata.sntp_asctime_r.*)
    *libesp-homekit.a:server.o(.rodata.*response* .rodata..L.str.*)
    *libcrypto.a:chacha.o(.rodata.*)
    *libcrypto.a:base64.o(.rodata.base64_table)
    *libmain.a:mem_manager.o(.rodata)
    *libnet80211.a:ieee80211_phy.o(*.irom.text*)
    *libcrypto.a:tweetnacl.o(.rodata.*)
    *libhttp-parser.a:http_parser.o(.rodata.normal_url_char .rodata.tokens .rodata.unhex)
    *(*.rodata *.rodata.SP* *.rodata.bigbyte *.rodata.cst* *.rodata.mon_lengths *.rodata.pc* *.rodata.tls_cipher* *.rodata..* *.rodata.phy_data *.rodata.mktime._DAYS_BEFORE_MONTH *.rodata.user_pre_init.at_partition_table *.irom.text*)
    *libesp-homekit.a:crypto.o(.rodata.*)
    *(*.rodata.K *.rodata.lfs_crc.*)
    *(*.rodata.[a-z]*)
    *(*.rodata.*)
    *(*.irom0.text.literal* *.literal*)
    *(*.irom0.text.* *.text*)
    *(.gnu.linkonce.r.*)
    __XT_EXCEPTION_TABLE__ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table)
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    *(.eh_frame)
    /*  C++ constructor and destructor tables, properly ordered:  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS__ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    . = ALIGN(4);		/* this table MUST be 4-byte aligned */
    _bss_table_start = ABSOLUTE(.);
    LONG(_bss_start)
    LONG(_bss_end)
    _bss_table_end = ABSOLUTE(.);

    _irom0_text_end = ABSOLUTE(.);
  } >irom0_0_seg :irom0_0_phdr

  .rodata : ALIGN(4)
  {
    _rodata_start = ABSOLUTE(.);
    _rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .bss ALIGN(8) (NOLOAD) : ALIGN(4)
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
    _heap_start = ABSOLUTE(.);
/*    _stack_sentry = ALIGN(0x8); */
  } >dram0_0_seg :dram0_0_bss_phdr
/* __stack = 0x3ffc8000; */

}

/* get ROM code address */
INCLUDE "../ld/eagle.rom.addr.v6.ld"

PROVIDE ( _xtos_exc_handler_table = 0x3fffc000 );
PROVIDE ( _xtos_c_handler_table = 0x3fffc100 );

PROVIDE ( ets_bit_task_priority = 0x3FFFC6FC );
PROVIDE ( ets_bit_count_task = 0x3FFFDAB8 );
PROVIDE ( ets_tab_task = 0x3FFFDAC0 );
