
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

#endif
