
#include "x86.h"
#include <stdio.h>
#include <stdlib.h>

void test_prefix_mask() {
  printf("=== Test Prefix Mask ===\n");
  prefix_mask_t mask = PREFIX_MASK_NONE;
  for(uint16_t word = 0x00;word != 0x100;word++) {
    prefix_mask_t m = get_prefix_mask((uint8_t)word);
    assert(!(mask & m)); // The same mask cannot be set twice
  }
  assert(mask == ((PREFIX_MASK_LAST << 1) - 1));
  printf("Pass!\n");
}

int main() {
  test_prefix_mask();
  return 0;
}