/*
chacha.h version 20080118
D. J. Bernstein
Public domain.
*/

#ifndef CHACHA_H
#define CHACHA_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

typedef struct
{
  u32 input[16]; /* could be compressed */
  /* 
   * [edit]
   *
   * Put here all state variable needed during the encryption process.
   */
} ECRYPT_ctx;

void ECRYPT_init(void);
void ECRYPT_keysetup(ECRYPT_ctx *x,const u8 *k,u32 kbits);
void ECRYPT_ivsetup(ECRYPT_ctx *x,const u8 *iv,u32 ivbits);
void ECRYPT_encrypt_bytes(ECRYPT_ctx *x,const u8 *m,u8 *c,u32 bytes);
void ECRYPT_decrypt_bytes(ECRYPT_ctx *x,const u8 *c,u8 *m,u32 bytes);
void ECRYPT_keystream_bytes(ECRYPT_ctx *x,u8 *stream,u32 bytes);

#endif
