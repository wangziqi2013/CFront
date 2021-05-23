
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

// Sets operand based on mode + r/m
void *parse_operand_mod_rm(operand_t *operand, int flags, int addr_mode, int rm, void *data) {
  if(addr_mode == ADDR_MODE_REG) {
    operand->operand_mode = OPERAND_REG;
    operand->reg = (flags & FLAG_W) ? gen_reg_16_table[rm] : gen_reg_8_table[rm]; // rm encodes a register
  } else {
    operand->operand_mode = OPERAND_MEM;
    operand->mem.addr_mode = addr_mode;
    if(addr_mode == ADDR_MODE_MEM_REG_ONLY) { 
      operand->mem.regs = addr_mode_reg_table_1[rm];
      // Directly addressed, followed by 16 bit absolute address
      // Note that this overrides the REG ONLY addressing mode
      if(rm == 6) {
        operand->mem.addr_mode = ADDR_MODE_MEM_DIRECT;
        operand->mem.disp16 = ptr_load_16(data);
        data = ptr_add_16(data);
      }
    } else if(addr_mode == ADDR_MODE_MEM_REG_DISP_8) {
      operand->mem.regs = addr_mode_reg_table_2[rm];
      operand->mem.disp8 = ptr_load_8(data);
      data = ptr_add_8(data);
    } else if(addr_mode == ADDR_MODE_MEM_REG_DISP_16) {
      operand->mem.regs = addr_mode_reg_table_2[rm];
      operand->mem.disp16 = ptr_load_16(data);
      data = ptr_add_16(data);
    }
  }
  return data;
}

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
  data = parse_operand_mod_rm(op, addr_mode, flags, rm, data);
  return data;
}

// Only parses mode + r/m and returns reg for the caller
void *parse_operand_1(operand_t *operand, uint32_t flags, int *reg, void *data) {
  uint8_t byte = ptr_load_8(data);
  data = ptr_add_8(data);
  int addr_mode = (int)(byte >> 6);
  *reg = (int)((byte >> 3) & 0x07);
  int rm = (int)(byte & 0x07);
  data = parse_operand_mod_rm(operand, addr_mode, flags, rm, data);
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

//* ins_t

const char *op_names[] = {
  "nop", "add", "push", "pop", "or", "addc", "sbb", "and", "daa", "sub", "das",
  "xor", "aaa", "cmp", "aas", "inc", "dec", 
  "jo"," jno", "jb", "jnb", "jz", "jnz", "jbe", "ja", "js", 
  "jns", "jpe", "jpo", "jl", "jge", "jle", "jg",
  "test", "xchg", "mov", "lea",
};

// ALU instructions occupy 6 opcodes, the first four being the general form
// (reg, mem/reg and four w/d combinations). The last two being 8-bit imm to AL
// and 16-bit imm to AX
void *parse_alu_ins(ins_t *ins, int diff, int op, void *data) {
  assert(diff >= 0 || diff <= 5);
  ins->op = op;
  switch(diff) {
    case 0x00: case 0x01: case 0x02: case 0x03: { // Two operands
      data = parse_operand_2(&ins->dest, &ins->src, ins->flags, data);
    } break;
    case 0x04: {
      operand_set_register(&ins->dest, REG_AL);
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0x05: {
      operand_set_register(&ins->dest, REG_AX);
      data = operand_set_imm_16(&ins->src, data);
    } break;
  }
  return data;
}

// ALU operation with src being imm and dest being 
void *parse_ins_grp1(ins_t *ins, void *data) {
  int reg = REG_NONE;
  // Parses mod + r/m operand as destination, and returns reg field
  data = parse_operand_1(&ins->dest, ins->flags, &reg, data);
  assert(reg >= 0 && reg <= 7);
  switch(reg) {
    case 0: ins->op = OP_ADD; break;
    case 1: ins->op = OP_OR; break;
    case 2: ins->op = OP_ADC; break;
    case 3: ins->op = OP_SBB; break;
    case 4: ins->op = OP_AND; break;
    case 5: ins->op = OP_SUB; break;
    case 6: ins->op = OP_XOR; break;
    case 7: ins->op = OP_CMP; break;
    default: assert(0); break;
  }
  // 0x82 just aliases 0x80, and 0x83 is imm8 and word argument???
  if(ins->opcode == 0x82 || ins->opcode == 0x83) {
    print_inst_addr(inst);
    error_exit("Unsupported opcode: 0x%X\n", ins->opcode);
  }
  // Read 16-bit or 8-bit immediate value
  data = (ins->flags & FLAG_W) ? operand_set_imm_16(&ins->src, data) : operand_set_imm_8(&ins->src, data);
  return data;
}

void *parse_ins(ins_t *ins, void *data) {
  void *old_data = data; // Compute size with this
  data = parse_prefix(ins, data);
  data = parse_opcode(ins, data);
  // Initialize operands
  ins->src.operand_mode = OPERAND_NONE;
  ins->dest.operand_mode = OPERAND_NONE;
  switch(ins->opcode) {
    case 0x90: ins->op = OP_NOP; break;
    case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: { // Two operand ADD
      data = parse_alu_ins(ins, ins->opcode - 0x00, OP_ADD, data);
    } break;
    case 0x06: {
      ins->op = OP_PUSH;
      operand_set_register(&ins->src, REG_ES);
    } break;
    case 0x07: {
      ins->op = OP_POP;
      operand_set_register(&ins->src, REG_ES);
    } break;
    case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: { // Two operand OR
      data = parse_alu_ins(ins, ins->opcode - 0x08, OP_OR, data);
    } break;
    case 0x0E: {
      ins->op = OP_PUSH;
      operand_set_register(&ins->src, REG_CS);
    } break;
    case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: { // Two operand ADC
      data = parse_alu_ins(ins, ins->opcode - 0x10, OP_ADC, data);
    } break;
    case 0x16: {
      ins->op = OP_PUSH;
      operand_set_register(&ins->src, REG_SS);
    } break;
    case 0x17: {
      ins->op = OP_POP;
      operand_set_register(&ins->src, REG_SS);
    } break;
    case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: { // Two operand SBB
      data = parse_alu_ins(ins, ins->opcode - 0x18, OP_SBB, data);
    } break;
    case 0x1E: {
      ins->op = OP_PUSH;
      operand_set_register(&ins->src, REG_DS);
    } break;
    case 0x1F: {
      ins->op = OP_POP;
      operand_set_register(&ins->src, REG_DS);
    } break;
    case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: { // Two operand AND
      data = parse_alu_ins(ins, ins->opcode - 0x20, OP_AND, data);
    } break;
    case 0x27: ins->op = OP_DAA; break;
    case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: { // Two operand SUB
      data = parse_alu_ins(ins, ins->opcode - 0x28, OP_SUB, data);
    } break;
    case 0x2F: ins->op = OP_DAS; break;
    case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: { // Two operand XOR
      data = parse_alu_ins(ins, ins->opcode - 0x30, OP_XOR, data);
    } break;
    case 0x37: ins->op = OP_AAA; break;
    case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: { // Two operand XOR
      data = parse_alu_ins(ins, ins->opcode - 0x38, OP_CMP, data);
    } break;
    case 0x3F: ins->op = OP_AAS; break;
    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: {
      ins->op = OP_INC;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x40]);
    } break;
    case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: {
      ins->op = OP_DEC;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x48]);
    } break;
    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: {
      ins->op = OP_PUSH;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x50]);
    } break;
    case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F: {
      ins->op = OP_POP;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x58]);
    } break;
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
    case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: { // Jump short
      ins->op = OP_JO + ins->opcode - 0x70; // Their codes are consecutive in op space
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0x80: case 0x81: case 0x82: case 0x83: {
      data = parse_ins_grp1(ins, data);
    } break;
    case 0x84: case 0x85: { // Order of dest and src does not matter
      ins->op = OP_TEST;
      data = parse_operand_2(&ins->dest, &ins->src, ins->flags, data);
    } break;
    case 0x86: case 0x87: { // Order of dest and src does not matter
      ins->op = OP_XCHG;
      data = parse_operand_2(&ins->dest, &ins->src, ins->flags, data);
    } break;
    case 0x88: case 0x89: case 0x8A: case 0x8B: { // mov
      ins->op = OP_MOV;
      data = parse_operand_2(&ins->dest, &ins->src, ins->flags, data);
    } break;
    case 0x8C: { // mov E, seg reg
      ins->op = OP_MOV;
      // Override the argument size to word (0x8C itself indicates byte)
      assert((ins->flags & FLAG_W) == 0);
      ins->flags |= FLAG_W;
      int reg;
      data = parse_operand_1(&ins->dest, ins->flags, &reg, data);
      assert(reg >= 0 && reg <= 3); // We only have 4 segment registers
      operand_set_register(&ins->src, seg_reg_table[reg]);
    } break;
    case 0x8D: { // LEA, dest is always 16-bit register, src is always addressing mode
      ins->op = OP_LEA;
      int reg;
      data = parse_operand_1(&ins->dest, ins->flags, &reg, data);
      assert(reg >= 0 && reg <= 7);
      if(ins->dest.operand_mode == OPERAND_REG) {
        print_inst_addr(inst);
        error_exit("LEA instruction must not have REG addressing mode\n");
      }
    } break;
    default: {
      print_inst_addr(inst);
      error_exit("Illegal opcode: 0x%X\n", ins->opcode);
    }
  }
  ins->size = (uint8_t)((uint64_t)data - (uint64_t)old_data);
  return data;
}
