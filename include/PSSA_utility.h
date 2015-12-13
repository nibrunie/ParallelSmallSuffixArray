#include <inttypes.h>


#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG
#define DEBUG_PRINTF printf
#else 
#define DEBUG_PRINTF 
#endif

#if 1
static inline uint64_t cycles()
{
  uint64_t t;
  __asm volatile ("rdtsc" : "=A"(t));
  return t;
}
/* x is a 64-bit unsigned value whose endianess
 * is swapped */
#define BYTE_SWAP(x) ((x >> 56) | \
  (((x >> 48) & 0xff) << 8) | \
  (((x >> 40) & 0xff) << 16) | \
  (((x >> 32) & 0xff) << 24) | \
  (((x >> 24) & 0xff) << 32) | \
  (((x >> 16) & 0xff) << 40) | \
  (((x >> 8) & 0xff) << 48) | \
  (((x) & 0xff) << 56))

#endif

#endif /* ndef __DEBUG_H__ */
