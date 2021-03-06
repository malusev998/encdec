#include <sodium/crypto_pwhash.h>
#include <sodium/crypto_secretstream_xchacha20poly1305.h>
#include <sodium/randombytes.h>
#include <stdio.h>
#include <string.h>

#include "../include/common.h"

static char hexconvtab[] = "0123456789abcdef";

int32_t bin2hex(char *const dst, const uint32_t dst_len, const byte *const src,
                const uint32_t src_len) {
  int32_t i, j;

  if (BIN2HEX_OUTPUT_BUFFER_LENGTH(src_len) > dst_len) {
    return ERROR_NOT_ENOUGH_BYTES;
  }

  for (i = j = 0; i < src_len; i++) {
    dst[j++] = hexconvtab[src[i] >> 4];
    dst[j++] = hexconvtab[src[i] & 15];
  }
  dst[j] = '\0';

  return i;
}

int32_t hex2bin(char *dst, uint32_t dst_len, const byte *const src,
                uint32_t src_len) {

  const uint32_t target_length = HEX2BIN_OUTPUT_BUFFER_LENGTH(src_len);
  int32_t i = 0, j = 0;
  byte d;
  if (target_length != dst_len) {
    return ERROR_NOT_ENOUGH_BYTES;
  }

  for (; i < target_length; i++) {
    byte c = src[j++];

    if (c >= '0' && c <= '9') {
      d = (c - '0') << 4;
    } else if (c >= 'a' && c <= 'f') {
      d = (c - 'a' + 10) << 4;
    } else if (c >= 'A' && c <= 'F') {
      d = (c - 'A' + 10) << 4;
    } else {
      return ERROR_INVALID_CHARACTER;
    }

    c = src[j++];
    if (c >= '0' && c <= '9') {
      d |= c - '0';
    } else if (c >= 'a' && c <= 'f') {
      d |= c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      d |= c - 'A' + 10;
    } else {
      return ERROR_INVALID_CHARACTER;
    }
    dst[i] = (char)d;
  }
  dst[i] = '\0';
  return i;
}

uint32_t hexdump(const FILE *const output, const char *const hex) {
#define LINE_LENGTH 70
  uint32_t i, count;
  char buffer[LINE_LENGTH];
  const char *hex_copy = hex;
  memset(buffer, 0, LINE_LENGTH);
  for (i = 0, count = 0; *hex_copy != 0; count++) {
    if (i >= LINE_LENGTH) {
      fprintf(output, "%s\n", buffer);
      memset(buffer, 0, LINE_LENGTH);
      fflush(output);
      i = 0;
    }
    buffer[i++] = '0';
    buffer[i++] = 'x';
    buffer[i++] = *hex_copy++;
    buffer[i++] = *hex_copy++;
    buffer[i++] = ' ';
  }

  fprintf(output, "%s\n", buffer);
  fflush(output);

#undef LINE_LENGTH
  return count;
}

int32_t generate_salt(byte *const out_salt) {
  if (!out_salt) {
    return -1;
  }

  randombytes_buf(out_salt, FILE_SALT_BYTES_LENGTH);
  return 0;
}

int32_t password_derivation(byte *out, byte *out_salt,
                            const char *const password) {

  return crypto_pwhash(
      out, DERIVED_PASSPHRASE_LENGTH, password, strlen(password), out_salt,
      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
      crypto_pwhash_ALG_DEFAULT);
}

#ifndef _MSC_VER
int32_t stricmp(const char *const p1, const char *const p2) {
  const byte *s1 = (const byte *)p1;
  const byte *s2 = (const byte *)p2;
  byte c1, c2;
  do {
    c1 = *s1++;
    c2 = *s2++;

    if (c1 > 64 && c1 < 91) {
      c1 += 32;
    }
    if (c2 > 64 && c2 < 91) {
      c2 += 32;
    }

    if (c1 == '\0')
      return c1 - c2;
  } while (c1 == c2);
  return c1 - c2;
}
#endif
