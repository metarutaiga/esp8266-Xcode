/* This linker script generated from xt-genldscripts.tpp for LSP . */
/* Linker Script for ld -N */
MEMORY
{
  dport0_0_seg :                        org = 0x3FF00000, len = 0x10
  dram0_0_seg :                         org = 0x3FFE8000, len = 0x14000
  iram1_0_seg :                         org = 0x40100000, len = 0x8000
  irom0_0_seg :                         org = 0x40201010, len = 0xE0000
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

  .irom0.text : ALIGN(4)
  {
    _irom0_text_start = ABSOLUTE(.);

    *(.sdk.version)

    /* esp-clang */
    *(*.irom0.pstr.*)

    *(.irom.literal .irom.text)
    *(.rodata.str*)

    *libcrypto.a:                       (.rodata .rodata.*)
    *liblwip_536.a:espconn_buf.o        (.rodata .rodata.*)
    *liblwip_536.a:etharp.o             (.rodata .rodata.*)
    *liblwip_536.a:sntp.o               (.rodata .rodata.*)
    *libmain.a:app_main.o               (.rodata .rodata.*)
    *libmain.a:mem_manager.o            (.rodata .rodata.*)
    *libnet80211.a:ieee80211_hostap.o   (.rodata .rodata.*)
    *libnet80211.a:ieee80211_ht.o       (.rodata .rodata.*)
    *libnet80211.a:ieee80211_input.o    (.rodata .rodata.* .data .data.*)
    *libnet80211.a:ieee80211_output.o   (.rodata .rodata.*)
    *libnet80211.a:wl_cnx.o             (.rodata .rodata.*)
    *libphy.a:phy_chip_v6.o             (.rodata .rodata.*)
    *libphy.a:phy_chip_v6_ana.o         (.rodata .rodata.*)
    *libphy.a:phy_chip_v6_cal.o         (.rodata .rodata.*)
    *libphy.a:phy_sleep.o               (.rodata .rodata.*)
    *libpp.a:esf_buf.o                  (.rodata .rodata.*)
    *libpp.a:lmac.o                     (.rodata .rodata.*)
    *libpp.a:rate_control.o             (.rodata .rodata.*)
    *libwpa.a:wpa.o                     (.rodata .rodata.*)

    /* esp-clang */
    *(*.irom.text.*)

    *libc.a:                            (.literal .text .literal.* .text.*)
    *libm.a:                            (.literal .text .literal.* .text.*)

    *libat.a:                           (.literal .text .literal.* .text.*)
    *libcrypto.a:                       (.literal .text .literal.* .text.*)
    *libespnow.a:                       (.literal .text .literal.* .text.*)
    *libjson.a:                         (.literal .text .literal.* .text.*)
    *liblwip.a:                         (.literal .text .literal.* .text.*)
    *liblwip_536.a:                     (.literal .text .literal.* .text.*)
    *libmesh.a:                         (.literal .text .literal.* .text.*)
    *libnet80211.a:                     (.literal .text .literal.* .text.*)
    *libsmartconfig.a:                  (.literal .text .literal.* .text.*)
    *libssl.a:                          (.literal .text .literal.* .text.*)
    *libupgrade.a:                      (.literal .text .literal.* .text.*)
    *libwpa.a:                          (.literal .text .literal.* .text.*)
    *libwpa2.a:                         (.literal .text .literal.* .text.*)
    *libwps.a:                          (.literal .text .literal.* .text.*)

    *(.irom.literal .irom.text .irom.literal.* .irom.text.* .irom0.literal .irom0.text .irom0.literal.* .irom0.text.*)

    *.c.o                               (.literal .text .literal.* .text.*)
    *.cc.o                              (.literal .text .literal.* .text.*)
    *.cpp.o                             (.literal .text .literal.* .text.*)

    . = ALIGN(4);		/* this table MUST be 4-byte aligned */
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

    . = ALIGN(4);		/* this table MUST be 4-byte aligned */
    _bss_table_start = ABSOLUTE(.);
    LONG(_bss_start)
    LONG(_bss_end)
    _bss_table_end = ABSOLUTE(.);

    _irom0_text_end = ABSOLUTE(.);
  } >irom0_0_seg :irom0_0_phdr

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
    *(.literal .text .literal.* .text.* .stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
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
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    *(SORT_BY_ALIGNMENT(EXCLUDE_FILE(*lib_a-impure.o) .data*))
    _data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rodata : ALIGN(4)
  {
    _rodata_start = ABSOLUTE(.);
    *(.sdk.version)
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    *libc.a:lib_a-impure.o(.data)
    __XT_EXCEPTION_TABLE__ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table)
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    *(.eh_frame)
    __XT_EXCEPTION_DESCS__ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
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

