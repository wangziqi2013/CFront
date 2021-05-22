
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

//* Register constants

#define REG_NONE      0
#define REG_AX        1
#define REG_BX        2
#define REG_CX        3
#define REG_DX        4
#define REG_SI        5
#define REG_DI        6
#define REG_BP        7
#define REG_SP        8

#define REG_AH        9
#define REG_AL        10
#define REG_BH        11
#define REG_BL        12
#define REG_CH        13
#define REG_CL        14
#define REG_DH        15
#define REG_DL        16

#define REG_CS        17
#define REG_DS        18
#define REG_ES        19
#define REG_SS        20

// The following will never be used by instructions but we add them anyways
#define REG_IP        21
#define REG_FLAGS     22

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
#define ADDR_MODE_REG              3

// Addressing mode
typedef struct {
  int addr_mode;         // Just copies the mode bits in the instruction
  addr_mode_reg_t regs;  // Register for addressing (one or two)
  union {
    uint8_t disp8;
    uint16_t disp16;
  };
} addr_mode_t;

// Operand type
#define OPERAND_REG     0
#define OPERAND_MEM     1
#define OPERAND_IMM8    2
#define OPERAND_IMM16   3
#define OPERAND_ADDR32  4

typedef struct {
  uint16_t offset;
  uint16_t seg;
} farptr_t;

// An operand can be either register or memory, which is encoded by addr_node_t
typedef struct {
  int operand_mode;
  union {
    int reg;
    addr_mode_t mem;
    uint16_t imm_16;     // 16 bit immediate value
    uint8_t imm_8;       // 8 bit immediate value
    farptr_t farptr;     // seg:offset full address
  };
} operand_t; 

inline static void *ptr_add_16(void *p) { return (void *)((uint16_t *)p + 1); }
inline static void *ptr_add_8(void *p) { return (void *)((uint8_t *)p + 1); }
inline static uint16_t ptr_load_16(void *p) { return *(uint16_t *)p; }
inline static uint8_t ptr_load_8(void *p) { return *(uint8_t *)p; }

// This is called after parsing opcode, d and w bit
void *parse_operands(operand_t *dest, operand_t *src, uint8_t byte, int d, int w, void *data);

// Instruction

typedef struct {
  farptr_t addr;
  int opcode;          // This does not include D and W flag
  uint32_t flags;
  operand_t dest;
  operand_t src;
} ins_t;

// This is called at the beginning of an instruction
void *parse_prefix(ins_t *ins, void *data);
void *parse_opcode(ins_t *ins, void *data);

#endif
