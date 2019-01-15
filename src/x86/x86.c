
#include "x86.h"

// Prefix related functions

// Returns the prefix mask given a byte; If not a mask return PREFIX_MASK_NONE
prefix_mask_t get_prefix_mask(uint8_t byte) {
  return PREFIX_MASK_NONE;
}