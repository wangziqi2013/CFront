
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
  SYSEXPECT(fp != NULL);
  int fwrite_ret = fwrite(s, strlen(s), 1, fp);
  SYSEXPECT(fwrite_ret == 1);
  fclose(fp);
  char command[512];
  snprintf(command, sizeof(command), "nasm -f bin -o %s %s", filename, temp_filename);
  int sys_ret = system(command);
  if(sys_ret != 0) {
    error_exit("nasm fails to return normally (exit code %d)\n", sys_ret);
  }
  snprintf(command, sizeof(command), "ndisasm -b 16 %s", filename);
  sys_ret = system(command);
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

void test_compile_helper() {
  TEST_BEGIN();
  char *test_str = "mov ax, 0x1234\nmov al, bl\nadd bx, [bp+si+0x2345]\njmp far [es:di+bp+0x23]\n";
  test_helper_compile(test_str, "test_compile_helper.bin");
  remove("test_compile_helper.bin");
  TEST_PASS();
  return;
}

// Test addr mode parsing and printing
void test_addr_mode() {
  TEST_BEGIN();
  uint8_t data[3] = {0x00, 0x56, 0x34};
  for(int i = 0;i < 256;i++) {
    data[0] = i;
    operand_t dest, src;
    parse_operand_2(&dest, &src, FLAG_W, data);
    operand_fprint(&dest, 0, stdout);
    fprintf(stdout, ", ");
    operand_fprint(&src, 0, stdout);
    putchar('\n');
  }
  TEST_PASS();
  return;
}

static void *test_helper_load_file(const char *filename, int *_size) {
  FILE *fp = fopen(filename, "r");
  SYSEXPECT(fp != NULL);
  int ret;
  ret = fseek(fp, 0, SEEK_END);
  SYSEXPECT(ret == 0);
  int size = (int)ftell(fp);
  SYSEXPECT(size != -1);
  if(size == 0) {
    error_exit("The file \"%s\" is empty\n", filename);
  }
  *_size = size;
  ret = fseek(fp, 0, SEEK_SET);
  SYSEXPECT(ret == 0);
  void *buf = malloc(size);
  SYSEXPECT(buf != NULL);
  ret = fread(buf, size, 1, fp);
  SYSEXPECT(ret == 1);
  return buf;
}

// Print a given file
static void test_helper_print_file(const char *filename) {
  int size = 0;
  void *data = test_helper_load_file("test_compile_helper.bin", &size);
  void *end = (uint8_t *)data + size;
  while(data < end) {
    ins_t ins;
    data = parse_ins(&ins, data);
    ins_fprint(&ins, stdout);
    fputc('\n', stdout);
  }
  assert(data == end);
  return;
}

static void test_ins(const char *test_str) {
  test_helper_compile(test_str, "test_compile_helper.bin");
  test_helper_print_file("test_compile_helper.bin");
  remove("test_compile_helper.bin");
  return;
}

void test_ins_fprint() {
  TEST_BEGIN();
  char *test_str = \
    "mov ax, 0x1234\n"
    "mov al, bl\n"
    "add bx, [bp+si+0x2345]\n"
    "jmp far [es:di+bp+0x23]\n"
    "call [bx+di]\n"
    "call [0x89ab]\n"
    "jmp 0x1234:0x5678"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

int main() {
  test_prefix_to_flag();
  test_compile_helper();
  test_addr_mode();
  test_ins_fprint();
  printf("All test passed!\n");
  return 0;
}
