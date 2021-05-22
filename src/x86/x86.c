
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
  int index = __builtin_ffsl(cmp) - 1;
  assert(index != -1);
  assert(index % 8 == 0);
  index = (index >> 3);
  return (uint32_t)0x1U << index;
}

uint32_t prefix_to_flag_scalar(uint8_t byte) {
  uint32_t flag = FLAG_NONE;
  switch(byte) {
    case PREFIX_REP: flag = FLAG_REP; break;
    case PREFIX_REPNE: flag = FLAG_REPNE; break;
    case PREFIX_CS: flag = FLAG_CS; break;
    case PREFIX_DS: flag = FLAG_DS; break;
    case PREFIX_ES: flag = FLAG_ES; break;
    case PREFIX_SS: flag = FLAG_SS; break;
    case PREFIX_LOCK: flag = FLAG_LOCK; break;
    default: break;
  }
  return flag;
}

//* R/M Tables

const int gen_reg_16_table[8] = {
  REG_AX, REG_CX, REG_DX, REG_BX, 
  REG_SP, REG_BP, REG_SI, REG_DI, 
};

const int gen_reg_8_table[8] = {
  REG_AL, REG_CL, REG_DL, REG_BL, 
  REG_AH, REG_CH, REG_DH, REG_BH, 
};

const int seg_reg_table[4] = {
  REG_ES, REG_CS, REG_SS, REG_DS, 
};

extern const rm_reg_t rm_table_1[8] = {
  {REG_BX, REG_SI}, {REG_BX, REG_DI}, {REG_BP, REG_SI}, {REG_BP, REG_DI}, 
  {REG_SI, REG_NONE}, {REG_DI, REG_NONE}, {REG_NONE, REG_NONE}, {REG_BX, REG_NONE}, 
};

