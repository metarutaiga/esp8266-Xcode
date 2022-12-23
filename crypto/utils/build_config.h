/*
 * wpa_supplicant/hostapd - Build time configuration defines
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This header file can be used to define configuration defines that were
 * originally defined in Makefile. This is mainly meant for IDE use or for
 * systems that do not have suitable 'make' tool. In these cases, it may be
 * easier to have a single place for defining all the needed C pre-processor
 * defines.
 */

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

/* Insert configuration defines, e.g., #define EAP_MD5, here, if needed. */

#ifdef CONFIG_WIN32_DEFAULTS
#define CONFIG_NATIVE_WINDOWS
#define CONFIG_ANSI_C_EXTRA
#define CONFIG_WINPCAP
#define IEEE8021X_EAPOL
#define PKCS12_FUNCS
#define PCSC_FUNCS
#define CONFIG_CTRL_IFACE
#define CONFIG_CTRL_IFACE_NAMED_PIPE
#define CONFIG_DRIVER_NDIS
#define CONFIG_NDIS_EVENTS_INTEGRATED
#define CONFIG_DEBUG_FILE
#define EAP_MD5
#define EAP_TLS
#define EAP_MSCHAPv2
#define EAP_PEAP
#define EAP_TTLS
#define EAP_GTC
#define EAP_OTP
#define EAP_LEAP
#define EAP_TNC
#define _CRT_SECURE_NO_DEPRECATE

#ifdef USE_INTERNAL_CRYPTO
#define CONFIG_TLS_INTERNAL_CLIENT
#define CONFIG_INTERNAL_LIBTOMMATH
#define CONFIG_CRYPTO_INTERNAL
#endif /* USE_INTERNAL_CRYPTO */
#endif /* CONFIG_WIN32_DEFAULTS */

#if defined(__XTENSA__)
#define CONFIG_CRYPTO_INTERNAL
#define CONFIG_INTERNAL_LIBTOMMATH
#define CONFIG_NO_STDOUT_DEBUG
#define CONFIG_SHA256
#define CONFIG_TLSV12
#define CONFIG_TLS_INTERNAL_CLIENT
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <machine/endian.h>
#define __BYTE_ORDER BYTE_ORDER
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __BIG_ENDIAN BIG_ENDIAN
#define bswap_16 __builtin_bswap16
#define bswap_32 __builtin_bswap32
#define bswap_64 __builtin_bswap64
#ifndef __cplusplus
#define __cplusplus
#include <c_types.h>
#undef __cplusplus
#endif
#include "osapi.h"
#include "os.h"
#undef os_malloc
#undef os_realloc
#undef os_free
#include "mem.h"
#define os_memcmp_const memcmp
#define aes_unwrap aes_unwrap_unused
#define rijndaelKeySetupDec rijndaelKeySetupDec_unused
#define rijndaelKeySetupEnc rijndaelKeySetupEnc_unused
#define Te0 Te0_unused
#define Td0 Td0_unused
#define Td4s Td4s_unused
#define rcons rcons_unused
#if defined(aes_internal_dec) || defined(aes_internal_enc)
#define aes_decrypt aes_decrypt_unused
#define aes_decrypt_deinit aes_decrypt_deinit_unused
#define aes_decrypt_init aes_decrypt_init_unused
#define aes_encrypt_init aes_encrypt_init_unused
#undef rijndaelKeySetupDec
#undef rijndaelKeySetupEnc
#undef Te0
#undef Td0
#undef Td4s
#undef rcons
#endif
#if defined(base64)
#define base64_decode base64_decode_unused
#define base64_encode base64_encode_unused
#endif
#if defined(md5)
#define hmac_md5 hmac_md5_unused
#define hmac_md5_vector hmac_md5_vector_unused
#endif
#if defined(md5_internal)
#define MD5Final MD5Final_unused
#define MD5Init MD5Init_unused
#define MD5Transform MD5Transform_unused
#define MD5Update MD5Update_unused
#define md5_vector md5_vector_unused
#endif
#if defined(rc4)
#define rc4_skip rc4_skip_unused
#endif
#if defined(sha1)
#define hmac_sha1 hmac_sha1_unused
#define hmac_sha1_vector hmac_sha1_vector_unused
#endif
#if defined(sha1_internal)
#define SHA1Final SHA1Final_unused
#define SHA1Init SHA1Init_unused
#define SHA1Transform SHA1Transform_unused
#define SHA1Update SHA1Update_unused
#define sha1_vector sha1_vector_unused
#endif
#if defined(sha1_pbkdf2)
#define pbkdf2_sha1 pbkdf2_sha1_unused
#endif
#if defined(sha1_prf)
#undef sha1_prf
#define sha1_prf sha1_prf_unused
#endif
#endif

#endif /* BUILD_CONFIG_H */
