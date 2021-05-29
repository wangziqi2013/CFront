
#ifndef _X86_H
#define _X86_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <assert.h>

//* util

// Error reporting and system call assertion
#define SYSEXPECT(expr) do { if(!(expr)) { perror(__func__); assert(0); exit(1); } } while(0)
#define error_exit(fmt, ...) do { fprintf(stderr, "%s error: " fmt, __func__, ##__VA_ARGS__); assert(0); exit(1); } while(0);
#ifndef NDEBUG
#define dbg_printf(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while(0);
#else
#define dbg_printf(fmt, ...) do {} while(0);
#endif

#define warn_printf(fmt, ...) do { fprintf(stdout, "Warning: " fmt, ##__VA_ARGS__); } while(0);

// Branching macro (this may have already been defined in other source files)
#ifndef likely
#define likely(x)       __builtin_expect((x),1)
#endif
#ifndef unlikely
#define unlikely(x)     __builtin_expect((x),0)
#endif

// Testing function print name and pass
#define TEST_BEGIN() do { printf("========== %s ==========\n", __func__); } while(0);
#define TEST_PASS() do { printf("Pass!\n"); } while(0);

// String functions
inline static char *strclone(const char *s) {
  int len = strlen(s);
  char *ret = (char *)malloc(len + 1);
  SYSEXPECT(ret != NULL);
  strcpy(ret, s);
  return ret;
}

//* Prefix (raw value in instructions)

#define PREFIX_REP    0xf3
#define PREFIX_REPE   PREFIX_REP
#define PREFIX_REPZ   PREFIX_REP

#define PREFIX_REPNE  0xf2
#define PREFIX_REPNZ  PREFIX_REPNE

// Segment override
#define PREFIX_CS     0x2e
#define PREFIX_DS     0x3e
#define PREFIX_ES     0x26
#define PREFIX_SS     0x36

#define PREFIX_LOCK   0xf0

// Used for MMX instruction
#define PREFIX_MMX_MASK 0xf0f036263e2ef2f3UL
// Covert prefix to a flag; Returns FLAG_NONE if not a prefix
uint32_t prefix_to_flag_mmx(uint8_t byte);
uint32_t prefix_to_flag_scalar(uint8_t byte);

#ifdef ENABLE_MMX
#define prefix_to_flag prefix_to_flag_mmx
#else 
#define prefix_to_flag prefix_to_flag_scalar
#endif

//* Global control

#define ENABLE_MMX

typedef struct {
  int warn_repeated_prefix; // Whether to warn repeated prefix bytes
} global_t;

extern global_t global;

//* Prefix flags

#define FLAG_NONE     0x00000000
#define FLAG_REP      0x00000001
#define FLAG_REPE     FLAG_REP
#define FLAG_REPZ     FLAG_REP

#define FLAG_REPNE    0x00000002
#define FLAG_REPNZ    FLAG_REPNE

#define FLAG_CS       0x00000004
#define FLAG_DS       0x00000008
#define FLAG_ES       0x00000010
#define FLAG_SS       0x00000020

#define FLAG_LOCK     0x00000040

// D flag in the opcode byte
#define FLAG_D        0x00000080
// W flag in the opcode byte
#define FLAG_W        0x00000100
// Whether call/jmp is far
#define FLAG_FAR      0x00000200

//* Register constants

#define REG_NONE      0

#define REG_BEGIN         1
#define REG_GEN_BEGIN     1
#define REG_GEN_16_BEGIN  1

#define REG_AX        1
#define REG_BX        2
#define REG_CX        3
#define REG_DX        4
#define REG_SI        5
#define REG_DI        6
#define REG_BP        7
#define REG_SP        8

#define REG_GEN_16_END    9
#define REG_GEN_8_BEGIN   9

#define REG_AH        9
#define REG_AL        10
#define REG_BH        11
#define REG_BL        12
#define REG_CH        13
#define REG_CL        14
#define REG_DH        15
#define REG_DL        16

#define REG_GEN_8_END 17
#define REG_GEN_END   17

#define REG_SEG_BEGIN 17

#define REG_CS        17
#define REG_DS        18
#define REG_ES        19
#define REG_SS        20

#define REG_SEG_END   21
#define REG_END       21

// The following will never be used by instructions but we add them anyways
#define REG_IP        21
#define REG_FLAGS     22

extern const char *reg_names[];

//* R/M Tables

// Maps REG field to register name, word size (w = 0)
extern const int gen_reg_16_table[8];
extern const int gen_reg_8_table[8];
extern const int seg_reg_table[4];

// Register pair for R/M addressing
typedef struct {
  int reg1;
  int reg2;
} addr_mode_reg_t;

// Mode = 00
extern const addr_mode_reg_t addr_mode_reg_table_1[8];
extern const addr_mode_reg_t addr_mode_reg_table_2[8];

#define ADDR_MODE_MEM_REG_ONLY     0
#define ADDR_MODE_MEM_REG_DISP_8   1
#define ADDR_MODE_MEM_REG_DISP_16  2
// This will cause the addr_mode object be not initialized
#define ADDR_MODE_REG              3
// This is not in the raw instruction
#define ADDR_MODE_MEM_DIRECT       4

// Addressing mode for memory operands
typedef struct {
  int addr_mode;         // ADDR_MODE_ macros
  addr_mode_reg_t regs;  // Register for addressing (one or two)
  union {
    uint8_t disp_8;
    uint16_t disp_16;
    uint16_t direct_addr; // Direct addressing mode uses this
  };
} addr_mode_t;

// Prints memory operand (ADDR_MODE_REG will not be printed because its encoding is not stored)
void addr_mode_fprint(addr_mode_t *addr_mode, uint32_t flags, FILE *fp);
// Generate the mode r/m byte and the following displacement bits
inline static uint8_t *addr_mode_gen(uint8_t mode, uint8_t reg, uint8_t rm, uint8_t *data) {
  assert(mode <= 3);
  assert(reg <= 7);
  assert(rm <= 7);
  data[0] = (mode << 6) | (reg << 3) | (rm);
  return data + 1;
}

// Operand type
#define OPERAND_NONE       0
#define OPERAND_REG        1
#define OPERAND_MEM        2
#define OPERAND_IMM_8      3
#define OPERAND_IMM_16     4
#define OPERAND_REL_8      5
#define OPERAND_REL_16     6
#define OPERAND_FARPTR     7
// The operand is a const value 1, which is not stored
#define OPERAND_IMPLIED_1  8

typedef struct {
  uint16_t offset;
  uint16_t seg;
} farptr_t;

// An operand can be either register or memory, which is encoded by addr_node_t
typedef struct {
  int operand_mode;
  union {
    int reg;             // Operand is in one of the registers (size implied by register width)
    addr_mode_t mem;     // Operand is in memory (size given by W flag)
    uint16_t imm_16;     // 16 bit immediate value
    uint8_t imm_8;       // 8 bit immediate value
    uint16_t rel_16;     // 16 bit relative
    uint16_t rel_8;      // 8 bit relative
    farptr_t farptr;     // seg:offset full address (32-bit operand)
  };
} operand_t; 

inline static void *ptr_add_16(void *p) { return (void *)((uint16_t *)p + 1); }
inline static void *ptr_add_8(void *p) { return (void *)((uint8_t *)p + 1); }
inline static uint16_t ptr_load_16(void *p) { return *(uint16_t *)p; }
inline static uint8_t ptr_load_8(void *p) { return *(uint8_t *)p; }

// Sets an operand as register. Register can be either general purpose or segment, but not IP or FLAGS
inline static void operand_set_register(operand_t *operand, int reg) {
  assert(reg >= REG_BEGIN && reg < REG_END);
  operand->operand_mode = OPERAND_REG;
  operand->reg = reg;
  return;
}

// Parse a 8-bit immediate value from the instruction stream
inline static void *operand_set_imm_8(operand_t *operand, void *data) {
  operand->operand_mode = OPERAND_IMM_8;
  operand->imm_8 = ptr_load_8(data);
  return ptr_add_8(data);
}

inline static void *operand_set_imm_16(operand_t *operand, void *data) {
  operand->operand_mode = OPERAND_IMM_16;
  operand->imm_16 = ptr_load_16(data);
  return ptr_add_16(data);
}

inline static void *operand_set_rel_8(operand_t *operand, void *data) {
  operand->operand_mode = OPERAND_REL_8;
  operand->rel_8 = ptr_load_8(data);
  return ptr_add_8(data);
}

inline static void *operand_set_rel_16(operand_t *operand, void *data) {
  operand->operand_mode = OPERAND_REL_16;
  operand->rel_16 = ptr_load_16(data);
  return ptr_add_16(data);
}

inline static void operand_set_const_8(operand_t *operand, uint8_t value) {
  operand->operand_mode = OPERAND_IMM_8;
  operand->imm_8 = value;
  return;
}

inline static void operand_set_const_16(operand_t *operand, uint16_t value) {
  operand->operand_mode = OPERAND_IMM_16;
  operand->imm_16 = value;
  return;
}

inline static void operand_set_implied_one(operand_t *operand) {
  operand->operand_mode = OPERAND_IMPLIED_1;
  return;
}

inline static void *operand_set_farptr(operand_t *operand, void *data) {
  operand->operand_mode = OPERAND_FARPTR;
  operand->farptr.offset = ptr_load_16(data);
  data = ptr_add_16(data);
  operand->farptr.seg = ptr_load_16(data);
  return ptr_add_16(data);
}

// Sets a direct memory operand
// This is specifically used by mov 0xA0 - 0xA3
inline static void *operand_set_mem_direct_addr(operand_t *operand, void *data) {
  operand->operand_mode = OPERAND_MEM;
  operand->mem.addr_mode = ADDR_MODE_MEM_DIRECT;
  operand->mem.direct_addr = ptr_load_16(data);
  return ptr_add_16(data);
}

// Given mode and r/m bits, set the operand
void *parse_operand_mod_rm(operand_t *operand, int addr_mode, int flags, int rm, void *data);
// Parsing 2 operands, must be either reg or mem
void *parse_operand_2(operand_t *dest, operand_t *src, uint32_t flags, void *data);
// Only parses mod + rm, returns REG
void *parse_operand_1(operand_t *operand, uint32_t flags, int *reg, void *data);

void operand_fprint(operand_t *operand, uint32_t flags, FILE *fp);

// Instruction

enum {
  OP_NOP = 0,
  OP_ADD,
  OP_PUSH,
  OP_POP,
  OP_OR,
  OP_ADC,
  OP_SBB,
  OP_AND,
  OP_DAA,
  OP_SUB,
  OP_DAS,
  OP_XOR,
  OP_AAA,
  OP_CMP,
  OP_AAS,
  OP_INC,
  OP_DEC,
  // Jump short
  OP_JO,
  OP_JNO,
  OP_JB,
  OP_JNB,
  OP_JZ,
  OP_JNZ,
  OP_JBE,
  OP_JA,
  OP_JS,
  OP_JNS,
  OP_JPE,
  OP_JPO,
  OP_JL,
  OP_JGE,
  OP_JLE,
  OP_JG,
  OP_TEST,
  OP_XCHG,
  OP_MOV,
  OP_LEA,
  OP_CBW,
  OP_CWD,
  OP_CALL,
  OP_WAIT,
  OP_PUSHF, 
  OP_POPF, 
  OP_SAHF,
  OP_LAHF,
  OP_MOVSB, 
  OP_MOVSW,
  OP_CMPSB,
  OP_CMPSW,
  OP_STOSB,
  OP_STOSW,
  OP_LODSB,
  OP_LODSW,
  OP_SCASB,
  OP_SCASW,
  OP_RET,
  OP_LES,
  OP_LDS,
  OP_RETF,
  OP_INT3,
  OP_INT,
  OP_INTO,
  OP_IRET,
  OP_ROL,
  OP_ROR,
  OP_RCL,
  OP_RCR,
  OP_SHL,
  OP_SHR,
  OP_SAR,
  OP_AAM,
  OP_AAD,
  OP_XLAT,
  OP_LOOPNZ,
  OP_LOOPZ,
  OP_LOOP,
  OP_JCXZ,
  OP_IN,
  OP_OUT,
  OP_JMP,
  OP_HLT,
  OP_CMC,
  OP_NOT,
  OP_NEG,
  OP_MUL,
  OP_IMUL,
  OP_DIV,
  OP_IDIV,
  OP_CLC,
  OP_STC,
  OP_CLI,
  OP_STI,
  OP_CLD,
  OP_STD,
};

// Maps op macros (see above) to string names
extern const char *op_names[];

typedef struct {
  farptr_t addr;       // Address of the instruction
  uint8_t opcode;      // This is the raw opcode byte includes D and W flag, i.e., it is the full 8 byte
  uint8_t op;          // This is the abstract operation (OP_ class)
  uint32_t flags;
  uint8_t size;        // Number of bytes in the instruction
  operand_t dest;
  operand_t src;       // If there only one operand, the src is used
} ins_t;

// Reads instructions from a file (used for debugging)
typedef struct {
  char *filename;              // File name
  void *data;                  // Content of the file
  void *end;                   // End pointer
  void *ptr;                   // Current read position
  int size;                    // File size (bytes)
  uint32_t next_addr;          // Next address of the instruction
} ins_reader_t;

ins_reader_t *ins_reader_init();
void ins_reader_free(ins_reader_t *ins_reader);
inline static int ins_reader_is_end(ins_reader_t *ins_reader) {
  return ins_reader->ptr >= ins_reader->end;
}
inline static int ins_reader_is_exact_end(ins_reader_t *ins_reader) {
  return ins_reader->ptr == ins_reader->end;
}
// The ins object is within the object
void ins_reader_next(ins_reader_t *ins_reader, ins_t *ins);

inline static void print_ins_addr(ins_t *ins) {
  fprintf(stderr, "Instruction at address %X:%X\n", ins->addr.seg, ins->addr.offset);
}

// This is called at the beginning of an instruction
void *parse_prefix(ins_t *ins, void *data);
void *parse_opcode(ins_t *ins, void *data);

void *parse_alu_ins(ins_t *ins, int diff, int op, void *data);
void *parse_ins_grp1(ins_t *ins, void *data);
void *parse_ins_grp2(ins_t *ins, void *data);
void *parse_ins_grp3(ins_t *ins, void *data);
void *parse_ins_grp4(ins_t *ins, void *data);
void *parse_ins_grp5(ins_t *ins, void *data);
void *parse_ins(ins_t *ins, void *data);

void ins_rel_8_fprint(ins_t *ins, uint32_t next_addr, FILE *fp);
void ins_rel_16_fprint(ins_t *ins, uint32_t next_addr, FILE *fp);
void ins_fprint(ins_t *ins, uint32_t next_addr, FILE *fp);

#endif
