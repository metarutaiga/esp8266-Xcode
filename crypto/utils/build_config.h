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
#define CONFIG_TLS_INTERNAL_CLIENT
#define CONFIG_INTERNAL_LIBTOMMATH
#define CONFIG_CRYPTO_INTERNAL
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"
#undef os_malloc
#undef os_realloc
#undef os_free
#define _C_TYPES_H_
#include "mem.h"
#define MD5Final MD5Final_unused
#define MD5Init MD5Init_unused
#define MD5Update MD5Update_unused
#define SHA1Final SHA1Final_unused
#define SHA1Init SHA1Init_unused
#define SHA1Transform SHA1Transform_unused
#define SHA1Update SHA1Update_unused
#define aes_decrypt aes_decrypt_unused
#define aes_decrypt_deinit aes_decrypt_deinit_unused
#define aes_decrypt_init aes_decrypt_init_unused
#define aes_unwrap aes_unwrap_unused
#define base64_decode base64_decode_unused
#define base64_encode base64_encode_unused
#define hmac_md5 hmac_md5_unused
#define hmac_md5_vector hmac_md5_vector_unused
#define hmac_sha1 hmac_sha1_unused
#define hmac_sha1_vector hmac_sha1_vector_unused
#define md5_vector md5_vector_unused
#define pbkdf2_sha1 pbkdf2_sha1_unused
#define rc4_skip rc4_skip_unused
#define rijndaelKeySetupDec rijndaelKeySetupDec_unused
#define rijndaelKeySetupEnc rijndaelKeySetupEnc_unused
#define sha1_prf sha1_prf_unused
#define sha1_vector sha1_vector_unused
#define Te0 Te0_unused
#define Td0 Td0_unused
#define Td4s Td4s_unused
#define rcons rcons_unused
#if defined(aes_internal_dec) || defined(aes_internal_enc)
#undef rijndaelKeySetupDec
#undef rijndaelKeySetupEnc
#undef Te0
#undef Td0
#undef Td4s
#undef rcons
#endif
#endif

#endif /* BUILD_CONFIG_H */
