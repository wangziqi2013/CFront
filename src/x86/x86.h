
#ifndef _X86_H
#define _X86_H

// Group 1
#define PREFIX_LOCK          0xF0
#define PREFIX_REPNE         0xF2
#define PREFIX_REP           0xF3
#define PREFIX_LOCK          0xF0
// Group 2
#define PREFIX_CS            0x2E
#define PREFIX_SS            0x36
#define PREFIX_DS            0x3E
#define PREFIX_ES            0x26
#define PREFIX_FS            0x64
#define PREFIX_GS            0x65
#define PREFIX_TAKEN         0x2E
#define PREFIX_NOT_TAKEN     0x3E
// Group 3
#define PREFIX_SIZE_OVERRIDE 0x66
// Group 4
#define PREFIX_ADDR_OVERRIDE 0x67

#endif