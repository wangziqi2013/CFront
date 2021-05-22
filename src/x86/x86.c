
#include "x86.h"

uint32_t prefix_to_flag_mmx(uint8_t byte) {
  // Load the mask value
  __m64 mask =_m_from_int64(PREFIX_MMX_MASK);
  // Broadcast input byte
  __m64 input = _mm_set1_pi8(byte);
  // Compare byte-by-byte, 0x00 if equal, 0xFF otherwise
  __m64 _cmp = _mm_cmpeq_pi8(mask, input);
  uint64_t cmp = _mm_cvtm64_si64(_cmp);
  if(cmp == 0UL) {
    return FLAG_NONE;
  }
  // Returns 1 + first index of "1"
  int index = __builtin_ffs(cmp) - 1;
  assert(index != -1);
  assert(index % 8 == 0);
  index = (index >> 3) + 1;
  return (uint32_t)index;
}