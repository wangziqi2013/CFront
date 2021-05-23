
#include "x86.h"

//* Global control

global_t global;

uint32_t prefix_to_flag_mmx(uint8_t byte) {
  // Load the mask value
  __m64 mask =_m_from_int64(PREFIX_MMX_MASK);
  // Broadcast input byte
  __m64 input = _mm_set1_pi8(byte);
  // Compare byte-by-byte, 0x00 if equal, 0xFF otherwise
  __m64 _cmp = _mm_cmpeq_pi8(mask, input);
  uint64_t cmp = _mm_cvtm64_si64(_cmp);
  if(cmp == 0UL) {
    return FLAG_NONE;
  }
  // Returns 1 + first index of "1"
  int index = __builtin_ffsl(cmp) - 1;
  assert(index != -1);
  assert(index % 8 == 0);
  index = (index >> 3);
  return (uint32_t)0x1U << index;
}

uint32_t prefix_to_flag_scalar(uint8_t byte) {
  uint32_t flag = FLAG_NONE;
  switch(byte) {
    case PREFIX_REP: flag = FLAG_REP; break;
    case PREFIX_REPNE: flag = FLAG_REPNE; break;
    case PREFIX_CS: flag = FLAG_CS; break;
    case PREFIX_DS: flag = FLAG_DS; break;
    case PREFIX_ES: flag = FLAG_ES; break;
    case PREFIX_SS: flag = FLAG_SS; break;
    case PREFIX_LOCK: flag = FLAG_LOCK; break;
    default: break;
  }
  return flag;
}

//* R/M Tables

const int gen_reg_16_table[8] = {
  REG_AX, REG_CX, REG_DX, REG_BX, 
  REG_SP, REG_BP, REG_SI, REG_DI, 
};

const int gen_reg_8_table[8] = {
  REG_AL, REG_CL, REG_DL, REG_BL, 
  REG_AH, REG_CH, REG_DH, REG_BH, 
};

const int seg_reg_table[4] = {
  REG_ES, REG_CS, REG_SS, REG_DS, 
};

const addr_mode_reg_t addr_mode_reg_table_1[8] = {
  {REG_BX, REG_SI}, {REG_BX, REG_DI}, {REG_BP, REG_SI}, {REG_BP, REG_DI}, 
  {REG_SI, REG_NONE}, {REG_DI, REG_NONE}, {REG_NONE, REG_NONE}, {REG_BX, REG_NONE}, 
};

const addr_mode_reg_t addr_mode_reg_table_2[8] = {
  {REG_BX, REG_SI}, {REG_BX, REG_DI}, {REG_BP, REG_SI}, {REG_BP, REG_DI}, 
  {REG_SI, REG_NONE}, {REG_DI, REG_NONE}, {REG_BP, REG_NONE}, {REG_BX, REG_NONE}, 
};

// Two operand instructions; Both operands must be either reg or mem
void *parse_operand_2(operand_t *dest, operand_t *src, uint32_t flags, void *data) {
  uint8_t byte = ptr_load_8(data);
  data = ptr_add_8(data);
  // Extract fields
  int addr_mode = (int)(byte >> 6);
  int reg = (int)((byte >> 3) & 0x07);
  int rm = (int)(byte & 0x07);
  // Parse register operand. If d  == 1 then register operand belongs to dest
  operand_t *op = (flags & FLAG_D) ? dest : src;
  op->operand_mode = OPERAND_REG;
  op->reg = (flags & FLAG_W) ? gen_reg_16_table[reg] : gen_reg_8_table[reg];
  // Parse r/m operand
  op = (flags & FLAG_D) ? src : dest;
  if(addr_mode == ADDR_MODE_REG) {
    op->operand_mode = OPERAND_REG;
    op->reg = (flags & FLAG_W) ? gen_reg_16_table[rm] : gen_reg_8_table[rm]; // rm encodes a register
  } else {
    op->operand_mode = OPERAND_MEM;
    op->mem.addr_mode = addr_mode;
    if(addr_mode == ADDR_MODE_MEM_REG_ONLY) { 
      op->mem.regs = addr_mode_reg_table_1[rm];
      // Directly addressed, followed by 16 bit absolute address
      if(rm == 6) {
        op->mem.disp16 = ptr_load_16(data);
        data = ptr_add_16(data);
      }
    } else if(addr_mode == ADDR_MODE_MEM_REG_DISP_8) {
      op->mem.regs = addr_mode_reg_table_2[rm];
      op->mem.disp8 = ptr_load_8(data);
      data = ptr_add_8(data);
    } else if(addr_mode == ADDR_MODE_MEM_REG_DISP_16) {
      op->mem.regs = addr_mode_reg_table_2[rm];
      op->mem.disp16 = ptr_load_16(data);
      data = ptr_add_16(data);
    }
  }
  return data;
}

void *parse_prefix(ins_t *ins, void *data) {
  while(1) {
    uint8_t byte = ptr_load_8(data);
    uint32_t flags = prefix_to_flag(byte);
    if(flags == FLAG_NONE) {
      break;
    }
    if(global.warn_repeated_prefix && (ins->flags & flags) != 0U) {
      warn_printf("Repeated prefix byte 0x%X at address %X:%X\n",
        byte, ins->addr.seg, ins->addr.offset);
    }
    ins->flags |= flags;
    data = ptr_add_8(data);
  }
  return data;
}

void *parse_opcode(ins_t *ins, void *data) {
  uint8_t byte = ptr_load_8(data);
  ins->opcode = byte;
  if(byte & 0x1) {
    ins->flags |= FLAG_W;
  } else if(byte & 0x1) {
    ins->flags |= FLAG_W;
  }
  return ptr_add_8(data);
}

void *parse_ins(ins_t *ins, void *data) {
  data = parse_prefix(ins, data);
  data = parse_opcode(ins, data);
  // Initialize operands
  ins->src.operand_mode = OPERAND_NONE;
  ins->dest.operand_mode = OPERAND_NONE;
  switch(ins->opcode) {
    case 0x90: ins->op = OP_NOP; break;
    case 0x00: case 0x01: case 0x02: case 0x03: { // Two operand add
      ins->op = OP_ADD;
      data = parse_operand_2(&ins->dest, ins->src, ins->flags, data);
    } break;
    case 0x04: {
      ins->op = OP_ADD;
      operand_set_register(&ins->dest, REG_AL);
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0x05: {
      ins->op = OP_ADD;
      operand_set_register(&ins->dest, REG_AX);
      data = operand_set_imm_16(&ins->src, data);
    } break;
    case 0x06: {
      ins->op = OP_PUSH;
      operand_set_register(&ins->dest, REG_ES);
    } break;
    case 0x07: {
      ins->op = OP_POP;
      operand_set_register(&ins->dest, REG_ES);
    } break;
    //case 0x
  }

  return data;
}
