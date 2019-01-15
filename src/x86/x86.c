
#include "x86.h"

// Prefix related functions

// The prefix mask is implied by the index of the entry in the table: (0x1 << index)
uint8_t prefix_code_table[] = {
  PREFIX_LOCK, PREFIX_REPNE, PREFIX_REP,
  PREFIX_CS, PREFIX_SS, PREFIX_DS, PREFIX_ES, PREFIX_FS, PREFIX_GS, PREFIX_TAKEN, PREFIX_NOT_TAKEN,
  PREFIX_SIZE_OVERRIDE, PREFIX_ADDR_OVERRIDE,
};

// Returns the prefix mask given a byte; If not a mask return PREFIX_MASK_NONE
prefix_mask_t get_prefix_mask(uint8_t byte) {
  return PREFIX_MASK_NONE;
}