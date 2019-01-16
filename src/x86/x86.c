
#include "x86.h"
#include "error.h"

// Stream related functions

// Returns the next byte in the stream. Reports error and exit if stream reaches to the end
uint8_t get_next_byte(x86_cxt_t *cxt) {
  if(iseof(cxt)) error_exit("Unexpected end of stream\n");
  return *cxt->p++;
}
uint8_t get_next_byte_noerr(x86_cxt_t *cxt) { return *cxt->p++; }
int iseof(x86_cxt_t *cxt) { return cxt->p == cxt->end_p; }

// Prefix related functions

// The prefix mask is implied by the index of the entry in the table: (0x1 << index)
uint8_t prefix_code_table[] = {
  PREFIX_LOCK, PREFIX_REPNE, PREFIX_REP,
  PREFIX_CS, PREFIX_SS, PREFIX_DS, PREFIX_ES, PREFIX_FS, PREFIX_GS, //PREFIX_TAKEN, PREFIX_NOT_TAKEN,
  PREFIX_SIZE_OVERRIDE, PREFIX_ADDR_OVERRIDE,
};

// Returns the prefix mask given a byte; If not a mask return PREFIX_MASK_NONE
prefix_mask_t get_prefix_mask(uint8_t byte) {
  for(unsigned int i = 0;i < sizeof(prefix_code_table) / sizeof(prefix_code_table[0]);i++) 
    if(byte == prefix_code_table[i]) return (prefix_mask_t)0x1 << i;
  return PREFIX_MASK_NONE;
}

// Return all prefixes in the stream. This function does not expect EOF
prefix_mask_t get_all_prefix_masks(x86_cxt_t *cxt) {
  prefix_mask_t ret, mask = PREFIX_MASK_NONE;
  while((ret = get_prefix_mask(get_next_byte(cxt))) != PREFIX_MASK_NONE) mask |= ret;
  return mask;
}