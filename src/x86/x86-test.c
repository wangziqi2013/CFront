
#include "x86.h"

void test_prefix_to_flag_mmx() {
  TEST_BEGIN();
  uint8_t byte = 0;
  for(int i = 0;i < 256;i++) {
    uint32_t flag = prefix_to_flag_mmx(byte);
    switch(byte) {
      case PREFIX_REP: assert(flag == FLAG_REP); break;
      case PREFIX_REPNE: assert(flag == FLAG_REPNE); break;
      case PREFIX_CS: assert(flag == FLAG_CS); break;
      case PREFIX_DS: assert(flag == FLAG_DS); break;
      case PREFIX_ES: assert(flag == FLAG_ES); break;
      case PREFIX_SS: assert(flag == FLAG_SS); break;
      case PREFIX_LOCK: assert(flag == FLAG_LOCK); break;
      default: assert(flag == FLAG_NONE); break;
    }
    byte++;
  }
  TEST_PASS();
  return;
}

int main() {
  test_prefix_to_flag_mmx();
  printf("All test passed!\n");
  return 0;
}
