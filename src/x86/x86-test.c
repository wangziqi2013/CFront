
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

// Print a given file
static void test_helper_print_file(const char *filename) {
  ins_reader_t *ins_reader = ins_reader_init(filename);
  while(ins_reader_is_end(ins_reader) == 0) {
    ins_t ins;
    ins_reader_next(ins_reader, &ins);
    ins_fprint(&ins, ins_reader->next_addr, stdout);
    fputc('\n', stdout);
  }
  assert(ins_reader_is_exact_end(ins_reader) == 1);
  ins_reader_free(ins_reader);
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

void test_00_05() {
  TEST_BEGIN();
  char *test_str = \
    "add [ss:bx], cl" "\n"
    "add [di+0x67], ax" "\n"
    "add ch, [bx+si+0x90]" "\n"
    "add dx, [bx]" "\n"
    "add al, 0x23" "\n"
    "add ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_06_07() {
  TEST_BEGIN();
  char *test_str = \
    "push es" "\n"
    "pop es" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_08_0d() {
  TEST_BEGIN();
  char *test_str = \
    "or [si+0x1234], cl" "\n"
    "or [di], ax" "\n"
    "or ch, [bp+di]" "\n"
    "or dx, [0x1234]" "\n"
    "or al, 0x23" "\n"
    "or ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_0e() {
  TEST_BEGIN();
  char *test_str = \
    "push cs" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_10_15() {
  TEST_BEGIN();
  char *test_str = \
    "adc [0x1234], ah" "\n"
    "adc [bp], sp" "\n"
    "adc dl, [bx]" "\n"
    "adc bp, [0x1234]" "\n"
    "adc al, 0x23" "\n"
    "adc ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_16_17() {
  TEST_BEGIN();
  char *test_str = \
    "push ss" "\n"
    "pop ss" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_18_1d() {
  TEST_BEGIN();
  char *test_str = \
    "sbb [0x1234], bh" "\n"
    "sbb [bp], cx" "\n"
    "sbb bh, [bx]" "\n"
    "sbb sp, [0x1234]" "\n"
    "sbb al, 0x23" "\n"
    "sbb ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_1e_1f() {
  TEST_BEGIN();
  char *test_str = \
    "push ds" "\n"
    "pop ds" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_20_25() {
  TEST_BEGIN();
  char *test_str = \
    "add [0x1234], bl" "\n"
    "add [bp], bx" "\n"
    "add ch, [bx]" "\n"
    "add dx, [0x1234]" "\n"
    "add al, 0x23" "\n"
    "add ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_27() {
  TEST_BEGIN();
  char *test_str = \
    "daa" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_28_2d() {
  TEST_BEGIN();
  char *test_str = \
    "sub [0x12], bl" "\n"
    "sub [bp+di+0x4567], bx" "\n"
    "sub ch, [bx+0x890a]" "\n"
    "sub dx, [0x1234]" "\n"
    "sub al, 0x23" "\n"
    "sub ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_2f() {
  TEST_BEGIN();
  char *test_str = \
    "das" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_30_35() {
  TEST_BEGIN();
  char *test_str = \
    "xor [cs:0x12], bl" "\n"
    "xor [bp+si+0x4567], bx" "\n"
    "xor ch, [bx+0xffff]" "\n"
    "xor dx, [0xffff]" "\n"
    "xor al, 0x23" "\n"
    "xor ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_37() {
  TEST_BEGIN();
  char *test_str = \
    "aaa" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_38_3d() {
  TEST_BEGIN();
  char *test_str = \
    "cmp [cs:0x12], bl" "\n"
    "cmp [bp+si+0x4567], bx" "\n"
    "cmp ch, [bx+0xffff]" "\n"
    "cmp dx, [0xffff]" "\n"
    "cmp al, 0x23" "\n"
    "cmp ax, 0x4567" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_3f() {
  TEST_BEGIN();
  char *test_str = \
    "aas" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_40_4f() {
  TEST_BEGIN();
  char *test_str = \
    "inc ax" "\n" "inc cx" "\n" "inc dx" "\n" "inc bx" "\n"
    "inc sp" "\n" "inc bp" "\n" "inc si" "\n" "inc di" "\n"
    "dec ax" "\n" "dec cx" "\n" "dec dx" "\n" "dec bx" "\n"
    "dec sp" "\n" "dec bp" "\n" "dec si" "\n" "dec di" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_50_5f() {
  TEST_BEGIN();
  char *test_str = \
    "push ax" "\n" "push cx" "\n" "push dx" "\n" "push bx" "\n"
    "push sp" "\n" "push bp" "\n" "push si" "\n" "push di" "\n"
    "pop ax" "\n" "pop cx" "\n" "pop dx" "\n" "pop bx" "\n"
    "pop sp" "\n" "pop bp" "\n" "pop si" "\n" "pop di" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_70_7f() {
  TEST_BEGIN();
  char *test_str = \
    "jo short 0x7f" "\n" "jno short 0xff" "\n" "jb short 0x80" "\n" "jnb short 0x102" "\n" 
    "jz short 0x01" "\n" "jnz short 0x02" "\n" 
    "jbe short 0x12" "\n" "ja short 0x23" "\n" "js short 0x34" "\n" "jns short 0x45" 
    "\n" "jpe short 0x56" "\n" "jpo short 0x67" "\n"
    "jl short 0x78" "\n" "jge short 0x89" "\n" "jle short 0x9a" "\n" "jg short 0xab" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
} 

// Group 1, op Eb, Ib
void test_80() {
  TEST_BEGIN();
  char *test_str = \
    "add bh, 0x12" "\n"
    "or bl, 0x34" "\n"
    "adc ch, 0x56" "\n"
    "sbb cl, 0x78" "\n"
    "and byte [0x1123], 0x9a" "\n"
    "sub byte [bx+si], 0xbc" "\n"
    "xor byte [bp+di+0x2345], 0xde" "\n"
    "cmp byte [bx+0x79], 0xff" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_81() {
  TEST_BEGIN();
  char *test_str = \
    "add bx, 0x1234" "\n"
    "or bp, 0x3456" "\n"
    "adc cx, 0x5678" "\n"
    "sbb dx, 0x789a" "\n"
    "and word [0x1123], 0x9aaa" "\n"
    "sub word [bx+si], 0xbccc" "\n"
    "xor word [bp+di+0x2345], 0xdeee" "\n"
    "cmp word [bx+0x79], 0xefff" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_83() {
  TEST_BEGIN();
  char *test_str = \
    "add bx, 0x12" "\n"
    "or bp, 0x34" "\n"
    "adc cx, 0x56" "\n"
    "sbb dx, 0x78" "\n"
    "and word [0x1123], 0xffff" "\n"
    "sub word [bx+si], 0xff8c" "\n"
    "xor word [bp+di+0x2345], 0xfff0" "\n"
    "cmp word [bx+0x79], 0x00" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_84_85() {
  TEST_BEGIN();
  char *test_str = \
    "test bl, ch" "\n"
    "test [0x1234], dl" "\n"
    "test [bx+di+0x12], ah" "\n"
    "test bx, cx" "\n"
    "test [0x1234], dx" "\n"
    "test [bx+di+0x12], bp" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_86_87() {
  TEST_BEGIN();
  char *test_str = \
    "xchg bl, ch" "\n"
    "xchg [0x1234], dl" "\n"
    "xchg [bx+di+0x12], ah" "\n"
    "xchg bx, cx" "\n"
    "xchg [0x1234], dx" "\n"
    "xchg [bx+di+0x12], bp" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_88_8e() {
  TEST_BEGIN();
  char *test_str = \
    "mov [0x12], ah" "\n"
    "mov [bx+si], dx" "\n"
    "mov cl, [bx+si]" "\n"
    "mov si, [bx+di+0x34]" "\n"
    // The following tests segment reg move
    "mov sp, cs" "\n"
    "mov ax, ds" "\n"
    "mov dx, es" "\n"
    "mov si, ss" "\n"
    // LEA
    "lea ax, [bx+di+0x890a]" "\n"
    "lea bx, [bp]" "\n"
    "lea si, [0x7654]" "\n"
    // Move gen reg to seg
    "mov cs, ax" "\n"
    "mov ds, bx" "\n"
    "mov es, dx" "\n"
    "mov ss, cx" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_8f() {
  TEST_BEGIN();
  char *test_str = \
    "pop ax" "\n"
    "pop cx" "\n"
    "pop si" "\n"
    "pop bp" "\n"
    "pop word [0x1234]" "\n"
    "pop word [bx]" "\n"
    "pop word [bx+0x9876]" "\n"
    "pop word [bp+si+0x76]" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_90() {
  TEST_BEGIN();
  char *test_str = \
    "nop" "\n"
    "nop" "\n"
    "nop" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_91_97() {
  TEST_BEGIN();
  char *test_str = \
    "xchg ax, bx" "\n"
    "xchg ax, cx" "\n"
    "xchg ax, dx" "\n"
    "xchg ax, si" "\n"
    "xchg ax, di" "\n"
    "xchg ax, sp" "\n"
    "xchg ax, bp" "\n"
    "xchg bx, ax" "\n"
    "xchg cx, ax" "\n"
    "xchg dx, ax" "\n"
    "xchg si, ax" "\n"
    "xchg di, ax" "\n"
    "xchg sp, ax" "\n"
    "xchg bp, ax" "\n"
    // Lock prefix
    "lock xchg ax, bx" "\n"
    "lock xchg ax, cx" "\n"
    "lock xchg sp, ax" "\n"
    "lock xchg bp, ax" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_98_99() {
  TEST_BEGIN();
  char *test_str = \
    "cbw\ncwd" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_9a() {
  TEST_BEGIN();
  char *test_str = \
    "call 0x1234:0x5678" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_9b_9f() {
  TEST_BEGIN();
  char *test_str = \
    "wait" "\n"
    "pushf" "\n"
    "popf" "\n"
    "sahf" "\n"
    "lahf" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_a0_a3() {
  TEST_BEGIN();
  char *test_str = \
    "mov al, [0x1134]" "\n"
    "mov ax, [0x1234]" "\n"
    "mov [0x2345], al" "\n"
    "mov [0x34], ax" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_a4_a7() {
  TEST_BEGIN();
  char *test_str = \
    "movsb" "\n"
    "movsw" "\n"
    "rep movsb" "\n"
    "rep movsw" "\n"
    "cmpsb" "\n"
    "cmpsw" "\n"
    "repz cmpsb" "\n"
    "repe cmpsw" "\n"
    "repnz cmpsb" "\n"
    "repne cmpsw" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_a8_a9() {
  TEST_BEGIN();
  char *test_str = \
    "test al, 0x34" "\n"
    "test ax, 0x9876" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_aa_af() {
  TEST_BEGIN();
  char *test_str = \
    "stosb" "\n"
    "stosw" "\n"
    "rep stosb" "\n"
    "rep stosw" "\n"
    "lodsb" "\n"
    "lodsw" "\n"
    "rep lodsb" "\n"
    "rep lodsw" "\n"
    "scasb" "\n"
    "scasw" "\n"
    "repz scasb" "\n"
    "repe scasw" "\n"
    "repnz scasb" "\n"
    "repne scasw" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_b0_bf() {
  TEST_BEGIN();
  char *test_str = \
    "mov al, 0x12" "\n"
    "mov cl, 0x12" "\n"
    "mov dl, 0x12" "\n"
    "mov bl, 0x12" "\n"
    "mov ah, 0x12" "\n"
    "mov ch, 0x12" "\n"
    "mov dh, 0x12" "\n"
    "mov bh, 0x12" "\n"
    "mov ax, 0x1234" "\n"
    "mov cx, 0x1234" "\n"
    "mov dx, 0x1234" "\n"
    "mov bx, 0x1234" "\n"
    "mov sp, 0x1234" "\n"
    "mov bp, 0x1234" "\n"
    "mov si, 0x1234" "\n"
    "mov di, 0x1234" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_c2_c3() {
  TEST_BEGIN();
  char *test_str = \
    "ret" "\n"
    "ret 0x9876" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_c4_c5() {
  TEST_BEGIN();
  char *test_str = \
    "les ax, [bx+si]" "\n"
    "lds cx, [0x1234]" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

// mov Eb, Ib and mov Ev, Iv
void test_c6_c7() {
  TEST_BEGIN();
  char *test_str = \
    "mov byte [bx+si], 0x98" "\n"
    "mov word [bp+di], 0x7654" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_ca_cb() {
  TEST_BEGIN();
  char *test_str = \
    "retf" "\n"
    "retf 0x7654" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_cc_cf() {
  TEST_BEGIN();
  char *test_str = \
    "int3" "\n"
    "int 0x255" "\n"
    "int 0x0" "\n"
    "into" "\n"
    "iret" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

// Group 2 operand, Eb, 1 or Ev, q
void test_d0_d1() {
  TEST_BEGIN();
  char *test_str = \
    "rol bl, 1" "\n"
    "ror al, 1" "\n"
    "rcl byte [bx], 1" "\n"
    "rcr byte [0x1234], 1" "\n"
    "shl byte [si], 1" "\n"
    "shr byte [di], 1" "\n"
    "sar byte [bp], 1" "\n"
    "rol bx, 1" "\n"
    "ror ax, 1" "\n"
    "rcl word [bx], 1" "\n"
    "rcr word [0x1234], 1" "\n"
    "shl word [si], 1" "\n"
    "shr word [di], 1" "\n"
    "sar word [bp], 1" "\n"
    // 0xD1, shift using CL
    "rol bl, cl" "\n"
    "ror al, cl" "\n"
    "rcl byte [bx], cl" "\n"
    "rcr byte [0x1234], cl" "\n"
    "shl byte [si], cl" "\n"
    "shr byte [di], cl" "\n"
    "sar byte [bp], cl" "\n"
    "rol bx, cl" "\n"
    "ror ax, cl" "\n"
    "rcl word [bx], cl" "\n"
    "rcr word [0x1234], cl" "\n"
    "shl word [si], cl" "\n"
    "shr word [di], cl" "\n"
    "sar word [bp], cl" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_d4_d5() {
  TEST_BEGIN();
  char *test_str = \
    "aam" "\n"
    "aam 0xa" "\n"
    "aam 0x7" "\n"
    "aad" "\n"
    "aad 0xa" "\n"
    "aad 0x7" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_d7() {
  TEST_BEGIN();
  char *test_str = \
    "xlat" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_e0_e3() {
  TEST_BEGIN();
  char *test_str = \
    "loopnz 0x23" "\n"
    "loopz 0x24" "\n"
    "loop 0x25" "\n"
    "label:" "\n"
    "loop label" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_e4_e7() {
  TEST_BEGIN();
  char *test_str = \
    "in al, 0x80" "\n"
    "in ax, 0x98" "\n"
    "out 0x56, al" "\n"
    "out 0x45, ax" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_e8_eb() {
  TEST_BEGIN();
  char *test_str = \
    "call 0x1234" "\n"
    "jmp 0x1237" "\n"
    "jmp 0x1234:0x5678" "\n"
    "jmp 0x45" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_ec_ef() {
  TEST_BEGIN();
  char *test_str = \
    "in al, DX" "\n"
    "in ax, DX" "\n"
    "out DX, al" "\n"
    "out DX, ax" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_f4_f5() {
  TEST_BEGIN();
  char *test_str = \
    "HLT" "\n"
    "CMC" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

// Group 3a and 3b
void test_f6_f7() {
  TEST_BEGIN();
  char *test_str = \
    "test byte [bx], 0x76" "\n"
    "not byte [bx]" "\n"
    "neg bl" "\n"
    "mul cl" "\n"
    "imul byte [bp+0x1123]" "\n"
    "div cl" "\n"
    "idiv byte [bp+0x1123]" "\n"
    // 0xf7
    "test word [bx], 0x76" "\n"
    "not word [bx]" "\n"
    "neg bx" "\n"
    "mul cx" "\n"
    "imul word [bp+0x1123]" "\n"
    "div dx" "\n"
    "idiv word [bp+0x1123]" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

void test_f8_fd() {
  TEST_BEGIN();
  char *test_str = \
    "clc" "\n"
    "stc" "\n"
    "cli" "\n"
    "sti" "\n"
    "cld" "\n"
    "std" "\n"
    ;
  test_ins(test_str);
  TEST_PASS();
  return;
}

// Grp4 and 5
void test_fe_ff() {
  TEST_BEGIN();
  char *test_str = \
    // Single memory operand
    "inc byte [bx]" "\n"
    "dec ah" "\n"
    "inc word [si]" "\n"
    "dec dh" "\n"
    "push word [di]" "\n"
    "push word [0x4567]" "\n"
    // call and jmp with memory operand (i.e., target address stored in memory)
    "call word [si + 0x5678]" "\n"
    "call far [si + 0x5678]" "\n"
    "jmp word [di + 0x5678]" "\n"
    "jmp far [bp + 0x5678]" "\n"
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
  // Exhaustive opcode test
  test_00_05();
  test_06_07();
  test_08_0d();
  test_0e();
  test_10_15();
  test_16_17();
  test_18_1d();
  test_1e_1f();
  test_20_25();
  test_27();
  test_28_2d();
  test_2f();
  test_30_35();
  test_37();
  test_38_3d();
  test_3f();
  test_40_4f();
  test_50_5f();
  test_70_7f();
  test_80();
  test_81();
  test_83();
  test_84_85();
  test_86_87();
  test_88_8e();
  test_8f();
  test_90();
  test_91_97();
  test_98_99();
  test_9a();
  test_9b_9f();
  test_a0_a3();
  test_a4_a7();
  test_a8_a9();
  test_aa_af();
  test_b0_bf();
  test_c2_c3();
  test_c4_c5();
  test_c6_c7();
  test_ca_cb();
  test_cc_cf();
  test_d0_d1();
  test_d4_d5();
  test_d7();
  test_e0_e3();
  test_e4_e7();
  test_e8_eb();
  test_ec_ef();
  test_f4_f5();
  test_f6_f7();
  test_f8_fd();
  test_fe_ff();
  printf("All test passed!\n");
  return 0;
}
