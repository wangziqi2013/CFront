
#ifndef _X86_H
#define _X86_H

#include <stdint.h>

// Group 1
#define PREFIX_LOCK          0xF0
#define PREFIX_REPNE         0xF2
#define PREFIX_REP           0xF3
// Group 2
#define PREFIX_CS            0x2E
#define PREFIX_SS            0x36
#define PREFIX_DS            0x3E
#define PREFIX_ES            0x26
#define PREFIX_FS            0x64
#define PREFIX_GS            0x65
//#define PREFIX_TAKEN         0x2E
//#define PREFIX_NOT_TAKEN     0x3E
// Group 3
#define PREFIX_SIZE_OVERRIDE 0x66
// Group 4
#define PREFIX_ADDR_OVERRIDE 0x67

// This represents no mask
#define PREFIX_MASK_NONE          0x00000000U
// Group 1 mask
#define PREFIX_MASK_LOCK          0x00000001U
#define PREFIX_MASK_REPNE         0x00000002U
#define PREFIX_MASK_REP           0x00000004U
// Group 2 mask
#define PREFIX_MASK_CS            0x00000008U
#define PREFIX_MASK_SS            0x00000010U
#define PREFIX_MASK_DS            0x00000020U
#define PREFIX_MASK_ES            0x00000040U
#define PREFIX_MASK_FS            0x00000080U
#define PREFIX_MASK_GS            0x00000100U
//#define PREFIX_MASK_TAKEN         0x00000200U
//#define PREFIX_MASK_NOT_TAKEN     0x00000400U
// Group 3 mask
#define PREFIX_MASK_SIZE_OVERRIDE 0x00000200U
// Group 4 mask
#define PREFIX_MASK_ADDR_OVERRIDE 0x00000400U
#define PREFIX_MASK_LAST          PREFIX_MASK_ADDR_OVERRIDE

extern uint8_t prefix_code_table[];

typedef uint32_t prefix_mask_t;
prefix_mask_t get_prefix_mask(uint8_t byte); 

#endif