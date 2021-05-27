
#include "x86.h"
#include <time.h>

// Compile (using nasm which must be installed) the given string into an output file
static void test_helper_compile(const char *s, const char *filename) {
  char temp_filename[256];
  if(strlen(filename) > 64) {
    error_exit("Output file name must be longer than 64 bytes");
  }
  snprintf(temp_filename, sizeof(temp_filename), "temp_in_%lu.asm", time(NULL));
  FILE *fp = fopen(temp_filename, "w");
  int fwrite_ret = fwrite(s, strlen(s), 1, fp);
  SYS_EXPECT(fwrite_ret == 1);
  fclose(fp);
  char command[512];
  snprintf(command, sizeof(command), "nasm -f bin -o %s %s", filename, temp_filename);
  int sys_ret = system(command);
  remove(temp_filename);
  return;
}

void test_prefix_to_flag() {
  TEST_BEGIN();
  uint8_t byte = 0;
  for(int i = 0;i < 256;i++) {
    uint32_t flag1 = prefix_to_flag_mmx(byte);
    uint32_t flag2 = prefix_to_flag_scalar(byte);
    assert(flag1 == flag2);
    switch(byte) {
      case PREFIX_REP: assert(flag1 == FLAG_REP); break;
      case PREFIX_REPNE: assert(flag1 == FLAG_REPNE); break;
      case PREFIX_CS: assert(flag1 == FLAG_CS); break;
      case PREFIX_DS: assert(flag1 == FLAG_DS); break;
      case PREFIX_ES: assert(flag1 == FLAG_ES); break;
      case PREFIX_SS: assert(flag1 == FLAG_SS); break;
      case PREFIX_LOCK: assert(flag1 == FLAG_LOCK); break;
      default: assert(flag1 == FLAG_NONE); break;
    }
    byte++;
  }
  TEST_PASS();
  return;
}

int main() {
  test_prefix_to_flag();
  printf("All test passed!\n");
  return 0;
}
