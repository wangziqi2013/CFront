
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

// Reg only, this also encoded direct addressing mode
const addr_mode_reg_t addr_mode_reg_table_1[8] = {
  {REG_BX, REG_SI}, {REG_BX, REG_DI}, {REG_BP, REG_SI}, {REG_BP, REG_DI}, 
  {REG_SI, REG_NONE}, {REG_DI, REG_NONE}, {REG_NONE, REG_NONE}, {REG_BX, REG_NONE}, 
};

// Reg + disp_8/disp_16
const addr_mode_reg_t addr_mode_reg_table_2[8] = {
  {REG_BX, REG_SI}, {REG_BX, REG_DI}, {REG_BP, REG_SI}, {REG_BP, REG_DI}, 
  {REG_SI, REG_NONE}, {REG_DI, REG_NONE}, {REG_BP, REG_NONE}, {REG_BX, REG_NONE}, 
};

const char *reg_names[] = {
  "None", "AX", "BX", "CX", "DX", "SI", "DI", "BP", "SP", 
  "AH", "AL", "BH", "BL", "CH", "CL", "DH", "DL", 
  "CS", "DS", "ES", "SS",
  "IP", "FLAGS",
};

void addr_mode_fprint(addr_mode_t *addr_mode, uint32_t flags, FILE *fp) {
  assert(addr_mode->addr_mode != ADDR_MODE_REG);
  fprintf(fp, "[");
  // nasm style, segment override is after the "[" character
  if(flags & FLAG_CS) {
    printf("cs:");
  } else if(flags & FLAG_DS) {
    printf("ds:");
  } else if(flags & FLAG_ES) {
    printf("es:");
  } else if(flags & FLAG_SS) {
    printf("ss:");
  }
  switch(addr_mode->addr_mode) {
    case ADDR_MODE_MEM_DIRECT: {
      fprintf(fp, "0x%04X", addr_mode->direct_addr);
    } break;
    case ADDR_MODE_MEM_REG_ONLY: {
      assert(addr_mode->regs.reg1 != REG_NONE);
      fprintf(fp, "%s", reg_names[addr_mode->regs.reg1]);
      if(addr_mode->regs.reg2 != REG_NONE) {
        fprintf(fp, "+%s", reg_names[addr_mode->regs.reg2]);
      }
    } break;
    case ADDR_MODE_MEM_REG_DISP_8: 
    case ADDR_MODE_MEM_REG_DISP_16: {
      assert(addr_mode->regs.reg1 != REG_NONE);
      fprintf(fp, "%s", reg_names[addr_mode->regs.reg1]);
      if(addr_mode->regs.reg2 != REG_NONE) {
        fprintf(fp, "+%s", reg_names[addr_mode->regs.reg2]);
      }
      // Print sign extended 8-bit displacement or 16-bit displacement
      fprintf(fp, addr_mode->addr_mode == ADDR_MODE_MEM_REG_DISP_8 ? "+0x%02X" : "+0x%04X", 
        (addr_mode->addr_mode == ADDR_MODE_MEM_REG_DISP_8) ? (int16_t)(int8_t)addr_mode->disp_8 : addr_mode->disp_16);
    } break;
  }
  fprintf(fp, "]");
  return;
}

// Sets operand based on mode + r/m
void *parse_operand_mod_rm(operand_t *operand, int addr_mode, int flags, int rm, void *data) {
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
        operand->mem.disp_16 = ptr_load_16(data);
        data = ptr_add_16(data);
      }
    } else if(addr_mode == ADDR_MODE_MEM_REG_DISP_8) {
      operand->mem.regs = addr_mode_reg_table_2[rm];
      operand->mem.disp_8 = ptr_load_8(data);
      data = ptr_add_8(data);
    } else if(addr_mode == ADDR_MODE_MEM_REG_DISP_16) {
      operand->mem.regs = addr_mode_reg_table_2[rm];
      operand->mem.disp_16 = ptr_load_16(data);
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
  assert(addr_mode >= 0 && addr_mode <= 3);
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
  assert(addr_mode >= 0 && addr_mode <= 3);
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
  if(byte & 0x01) {
    ins->flags |= FLAG_W;
  }
  if(byte & 0x02) {
    ins->flags |= FLAG_D;
  }
  return ptr_add_8(data);
}

// Only segment override prefixes in the flags are used
// Note that this function does not print rel_8 and rel_16 since they require instruction information
void operand_fprint(operand_t *operand, uint32_t flags, FILE *fp) {
  switch(operand->operand_mode) {
    case OPERAND_REG: {
      fprintf(fp, "%s", reg_names[operand->reg]);
    } break;
    case OPERAND_MEM: {
      addr_mode_fprint(&operand->mem, flags, fp);
    } break;
    case OPERAND_IMM_8: {
      fprintf(fp, "0x%02X", operand->imm_8);
    } break;
    case OPERAND_IMM_16: {
      fprintf(fp, "0x%04X", operand->imm_16);
    } break;
    case OPERAND_FARPTR: { // Only used by mov
      fprintf(fp, "0x%04X:0x%04X", operand->farptr.seg, operand->farptr.offset);
    } break;
    case OPERAND_IMPLIED_1: { // Implied const value 1
      fprintf(fp, "1");
    } break;  
    default: {
      error_exit("(Internal error) Unknown operand mode: %d\n", operand->operand_mode);
    }
  }
  return;
}

//* ins_t

const char *op_names[] = {
  "nop", "add", "push", "pop", "or", "adc", "sbb", "and", "daa", "sub", "das",
  "xor", "aaa", "cmp", "aas", "inc", "dec", 
  "jo", "jno", "jb", "jnb", "jz", "jnz", "jbe", "ja", "js", 
  "jns", "jpe", "jpo", "jl", "jge", "jle", "jg",
  "test", "xchg", "mov", "lea", "cbw", "cwd", "call",
  "wait", "pushf", "popf", "sahf", "lahf",
  "movsb", "movsw", "cmpsb", "cmpsw",
  "stosb", "stosw", "lodsb", "lodsw", "scasb", "scasw",
  "ret", "les", "lds", "retf", "int3", "int", "into", "iret",
  "rol", "ror", "rcl", "rcr", "shl", "shr", "sar",
  "aam", "aad", "xlat",
  "loopnz", "loopz", "loop", "jcxz",
  "in", "out", "jmp", "hlt", "cmc",
  "not", "neg", "mul", "imul", "div", "idiv",
  "clc", "stc", "cli", "sti", "cld", "std",
};

// ALU instructions occupy 6 opcodes, the first four being the general form
// (reg, mem/reg and four w/d combinations). The last two being 8-bit imm to AL
// and 16-bit imm to AX
void *parse_alu_ins(ins_t *ins, int diff, int op, void *data) {
  assert(diff >= 0 || diff <= 5);
  ins->op = op;
  //printf("flags %X opcode %X\n", ins->flags, ins->opcode);
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
  // This words for 0x83, i.e., it parses a word destination
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
  // 0x82 just aliases 0x80
  if(ins->opcode == 0x82) {
    print_ins_addr(ins);
    error_exit("Unsupported opcode: 0x%X\n", ins->opcode);
  }
  // Read 16-bit or 8-bit immediate value
  if(ins->opcode != 0x83) {
    data = (ins->flags & FLAG_W) ? operand_set_imm_16(&ins->src, data) : operand_set_imm_8(&ins->src, data);
  } else {
    // 0x83 always have 8-bit immediate source and 16-bit destination
    data = operand_set_imm_8(&ins->src, data);
  }
  return data;
}

void *parse_ins_grp2(ins_t *ins, void *data) {
  int reg = REG_NONE;
  // Parses mod + r/m operand as destination, and returns reg field
  data = parse_operand_1(&ins->dest, ins->flags, &reg, data);
  assert(reg >= 0 && reg <= 7);
  // Set source operand
  switch(ins->opcode) {
    case 0xD0: operand_set_implied_one(&ins->src); break;
    case 0xD1: operand_set_implied_one(&ins->src); break;
    case 0xD2: operand_set_register(&ins->src, REG_CL); break;
    case 0xD3: operand_set_register(&ins->src, REG_CL); break;
    default: assert(0); break;
  }
  switch(reg) {
    case 0: ins->op = OP_ROL; break;
    case 1: ins->op = OP_ROR; break;
    case 2: ins->op = OP_RCL; break;
    case 3: ins->op = OP_RCR; break;
    case 4: ins->op = OP_SHL; break;
    case 5: ins->op = OP_SHR; break;
    case 7: ins->op = OP_SAR; break;
    default: { // reg == 6 is invalid
      print_ins_addr(ins);
      error_exit("Unknown reg field (2nd opcode) for grp2: %X\n", reg);
    } break;
  }
  return data;
}

// 0xF6, 0xF7, flags is set accordingly (byte argument)
void *parse_ins_grp3(ins_t *ins, void *data) {
  int reg = REG_NONE;
  // Parses mod + r/m operand as src, and returns reg field
  // Note that this works for both 0xF6 (Grp3a) and 0xF7 (Grp3b)
  data = parse_operand_1(&ins->src, ins->flags, &reg, data);
  assert(reg >= 0 && reg <= 7);
  switch(reg) {
    case 0: { // Test Ev, Ib/Iw
      ins->op = OP_TEST;
      // It is destination
      ins->dest = ins->src;
      data = (ins->opcode == 0xF6) ? operand_set_imm_8(&ins->src, data) : operand_set_imm_16(&ins->src, data);
    } break;
    case 2: ins->op = OP_NOT; break;
    case 3: ins->op = OP_NEG; break;
    case 4: ins->op = OP_MUL; break;
    case 5: ins->op = OP_IMUL; break;
    case 6: ins->op = OP_DIV; break;
    case 7: ins->op = OP_IDIV; break;
    default: { // reg == 1 is invalid
      print_ins_addr(ins);
      error_exit("Unknown reg field (2nd opcode) for grp3: %X\n", reg);
    } break;
  }
  return data;
}

void *parse_ins_grp4(ins_t *ins, void *data) {
  int reg = REG_NONE;
  // Parses mod + r/m operand as src, and returns reg field
  data = parse_operand_1(&ins->src, ins->flags, &reg, data);
  assert(reg >= 0 && reg <= 7);
  switch(reg) {
    case 0: ins->op = OP_INC; break;
    case 1: ins->op = OP_DEC; break;
    default: { // reg == 2 - 7 is invalid
      print_ins_addr(ins);
      error_exit("Unknown reg field (2nd opcode) for grp4: %X\n", reg);
    } break;
  }
  return data;
}

void *parse_ins_grp5(ins_t *ins, void *data) {
  int reg = REG_NONE;
  // Parses mod + r/m operand as src, and returns reg field. This will read word size operand
  data = parse_operand_1(&ins->src, ins->flags, &reg, data);
  assert(reg >= 0 && reg <= 7);
  switch(reg) {
    case 0: ins->op = OP_INC; break;
    case 1: ins->op = OP_DEC; break;
    case 2: ins->op = OP_CALL; break; // Call near, absolute indirect
    case 3: { // Call far, offset:seg stored in the given memory location
      ins->op = OP_CALL; 
      if(ins->src.operand_mode != OPERAND_MEM) {
        print_ins_addr(ins);
        error_exit("Call (FF/3) must always have memory operand");
      }
      ins->flags |= FLAG_FAR;
    } break;
    case 4: ins->op = OP_JMP; break; // Jmp near absolute indirect
    case 5: {
      ins->op = OP_JMP;
      if(ins->src.operand_mode != OPERAND_MEM) {
        print_ins_addr(ins);
        error_exit("Jmp (FF/5) must always have memory operand");
      }
      ins->flags |= FLAG_FAR;
    } break;
    case 6: ins->op = OP_PUSH; break;
    default: { // reg == 7 is invalid
      print_ins_addr(ins);
      error_exit("Unknown reg field (2nd opcode) for grp4: %X\n", reg);
    } break;
  }
  return data;
}

void *parse_ins(ins_t *ins, void *data) {
  void *old_data = data; // Compute size with this
  ins->flags = 0;
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
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, REG_ES);
    } break;
    case 0x07: {
      ins->op = OP_POP;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, REG_ES);
    } break;
    case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: { // Two operand OR
      data = parse_alu_ins(ins, ins->opcode - 0x08, OP_OR, data);
    } break;
    case 0x0E: {
      ins->op = OP_PUSH;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, REG_CS);
    } break;
    case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: { // Two operand ADC
      data = parse_alu_ins(ins, ins->opcode - 0x10, OP_ADC, data);
    } break;
    case 0x16: {
      ins->op = OP_PUSH;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, REG_SS);
    } break;
    case 0x17: {
      ins->op = OP_POP;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, REG_SS);
    } break;
    case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: { // Two operand SBB
      data = parse_alu_ins(ins, ins->opcode - 0x18, OP_SBB, data);
    } break;
    case 0x1E: {
      ins->op = OP_PUSH;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, REG_DS);
    } break;
    case 0x1F: {
      ins->op = OP_POP;
      ins->flags |= FLAG_W;
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
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x40]);
    } break;
    case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: {
      ins->op = OP_DEC;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x48]);
    } break;
    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: {
      ins->op = OP_PUSH;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x50]);
    } break;
    case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F: {
      ins->op = OP_POP;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x58]);
    } break;
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
    case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: { // Jump short
      ins->op = OP_JO + ins->opcode - 0x70; // Their codes are consecutive in op space
      data = operand_set_rel_8(&ins->src, data); // Use relative value
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
      data = parse_operand_1(&ins->src, ins->flags, &reg, data); // Source must be memory address
      assert(reg >= 0 && reg <= 7);
      if(ins->src.operand_mode != OPERAND_MEM) {
        print_ins_addr(ins);
        error_exit("%s instruction must have memory type operand\n", op_names[ins->op]);
      }
      operand_set_register(&ins->dest, gen_reg_16_table[reg]); // Dest must be 16-bit gen purpose register
    } break;
    case 0x8E: { // Move to seg register from r/m
      ins->op = OP_MOV;
      // Override the argument size to word (0x8C itself indicates byte)
      assert((ins->flags & FLAG_W) == 0);
      ins->flags |= FLAG_W;
      int reg;
      data = parse_operand_1(&ins->src, ins->flags, &reg, data);
      assert(reg >= 0 && reg <= 3); // We only have 4 segment registers
      operand_set_register(&ins->dest, seg_reg_table[reg]);
    } break;
    case 0x8F: { // Pop to r/m, ignore D bit, note that single operand operation have src as operand
      ins->op = OP_POP;
      int reg; // Ignore this
      data = parse_operand_1(&ins->src, ins->flags, &reg, data);
      (void)reg;
    } break;
    case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: { // XCHG with AX
      ins->op = OP_XCHG;
      ins->flags |= FLAG_W;
      operand_set_register(&ins->dest, REG_AX);
      // Note that we skip AX, so the index start at 1 (base 0x90)
      operand_set_register(&ins->src, gen_reg_16_table[ins->opcode - 0x90]);
    } break;
    case 0x98: ins->op = OP_CBW; break; // Byte size
    case 0x99: ins->op = OP_CWD; break; // Word size
    case 0x9A: { // call far immediate far address
      ins->op = OP_CALL;
      ins->flags |= FLAG_FAR;
      data = operand_set_farptr(&ins->src, data);
    } break;
    case 0x9B: ins->op = OP_WAIT; break; // This may be treated as a prefix, but we encode it as an inst
    case 0x9C: ins->op = OP_PUSHF; break;
    case 0x9D: ins->op = OP_POPF; break;
    case 0x9E: ins->op = OP_SAHF; break;
    case 0x9F: ins->op = OP_LAHF; break;
    case 0xA0: { // mov AL, [offset]
      ins->op = OP_MOV;
      operand_set_register(&ins->dest, REG_AL);
      data = operand_set_mem_direct_addr(&ins->src, data);
    } break;
    case 0xA1: { // mov AX, [offset]
    ins->op = OP_MOV;
      operand_set_register(&ins->dest, REG_AX);
      data = operand_set_mem_direct_addr(&ins->src, data);
    } break;
    case 0xA2: { // mov [offset], AL
      ins->op = OP_MOV;
      data = operand_set_mem_direct_addr(&ins->dest, data);
      operand_set_register(&ins->src, REG_AL);
    } break;
    case 0xA3: { // mov [offset], AX
      ins->op = OP_MOV;
      data = operand_set_mem_direct_addr(&ins->dest, data);
      operand_set_register(&ins->src, REG_AX);
    } break;
    case 0xA4: ins->op = OP_MOVSB; break;
    case 0xA5: ins->op = OP_MOVSW; break;
    case 0xA6: ins->op = OP_CMPSB; break;
    case 0xA7: ins->op = OP_CMPSW; break;
    case 0xA8: {
      ins->op = OP_TEST;
      operand_set_register(&ins->dest, REG_AL);
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0xA9: {
      ins->op = OP_TEST;
      operand_set_register(&ins->dest, REG_AX);
      data = operand_set_imm_16(&ins->src, data);
    } break;
    case 0xAA: ins->op = OP_STOSB; break;
    case 0xAB: ins->op = OP_STOSW; break;
    case 0xAC: ins->op = OP_LODSB; break;
    case 0xAD: ins->op = OP_LODSW; break;
    case 0xAE: ins->op = OP_SCASB; break;
    case 0xAF: ins->op = OP_SCASW; break;
    case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7: {
      ins->op = OP_MOV;
      ins->flags &= (~FLAG_W); // Clear word flag
      operand_set_register(&ins->dest, gen_reg_8_table[ins->opcode - 0xB0]);
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF: {
      ins->op = OP_MOV;
      ins->flags |= FLAG_W; // Set word flag
      operand_set_register(&ins->dest, gen_reg_16_table[ins->opcode - 0xB8]);
      data = operand_set_imm_16(&ins->src, data);
    } break;
    case 0xC2: {
      ins->op = OP_RET;
      ins->flags |= FLAG_W;
      data = operand_set_imm_16(&ins->src, data);
    } break;
    case 0xC3: ins->op = OP_RET; break;
    case 0xC4: case 0xC5: { // LES and LDS
      ins->op = (ins->opcode == 0xC4) ? OP_LES : OP_LDS;
      ins->flags |= FLAG_W; // Set word flag
      int reg;
      data = parse_operand_1(&ins->src, ins->flags, &reg, data);
      assert(reg >= 0 && reg <= 7);
      if(ins->src.operand_mode != OPERAND_MEM) {
        print_ins_addr(ins);
        error_exit("%s must have memory operand\n", op_names[ins->op]);
      }
      operand_set_register(&ins->dest, gen_reg_16_table[reg]);
    } break;
    case 0xC6: case 0xC7: { // mov r/m, imm_8/imm_16
      ins->op = OP_MOV;
      int reg;
      data = parse_operand_1(&ins->dest, ins->flags, &reg, data);
      (void)reg;
      data = (ins->opcode == 0xC6) ? operand_set_imm_8(&ins->src, data) : operand_set_imm_16(&ins->src, data);
    } break;
    case 0xCA: {
      ins->op = OP_RETF;
      ins->flags |= FLAG_W;
      data = operand_set_imm_16(&ins->src, data);
    } break;
    case 0xCB: ins->op = OP_RETF; break;
    case 0xCC: { // INT3
      ins->op = OP_INT3; 
      //operand_set_const_8(&ins->src, 0x3);
    } break;
    case 0xCD: { // INT imm8
      ins->op = OP_INT; 
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0xCE: ins->op = OP_INTO; break;
    case 0xCF: ins->op = OP_IRET; break;
    case 0xD0: case 0xD1: case 0xD2: case 0xD3: {
      data = parse_ins_grp2(ins, data);
    } break;
    case 0xD4: {
      ins->op = OP_AAM;
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0xD5: {
      ins->op = OP_AAD;
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0xD7: ins->op = OP_XLAT; break;
    case 0xE0: case 0xE1: case 0xE2: case 0xE3: {
      ins->op = OP_LOOPNZ + (ins->opcode - 0xE0);
      data = operand_set_rel_8(&ins->src, data);
    } break;
    case 0xE4: case 0xE5: { // IN AL/AX, imm8
      ins->op = OP_IN;
      operand_set_register(&ins->dest, (ins->opcode == 0xE4) ? REG_AL : REG_AX);
      data = operand_set_imm_8(&ins->src, data);
    } break;
    case 0xE6: case 0xE7: { // OUT imm8, AL/AX
      ins->op = OP_OUT;
      data = operand_set_imm_8(&ins->dest, data);
      operand_set_register(&ins->src, (ins->opcode == 0xE6) ? REG_AL : REG_AX);
    } break;
    case 0xE8: { // Call rel16 in imm16
      ins->op = OP_CALL;
      data = operand_set_rel_16(&ins->src, data);
    } break;
    case 0xE9: { // jmp rel16 in imm16
      ins->op = OP_JMP;
      data = operand_set_rel_16(&ins->src, data);
    } break;
    case 0xEA: {  // jmp farptr in imm32
      ins->op = OP_JMP;
      data = operand_set_farptr(&ins->src, data);
    } break;
    case 0xEB: {  // jmp rel8
      ins->op = OP_JMP;
      data = operand_set_rel_8(&ins->src, data);
    } break;
    case 0xEC: {  // in AL, DX
      ins->op = OP_IN;
      operand_set_register(&ins->dest, REG_AL);
      operand_set_register(&ins->src, REG_DX);
    } break;
    case 0xED: {  // in AX, DX
      ins->op = OP_IN;
      operand_set_register(&ins->dest, REG_AX);
      operand_set_register(&ins->src, REG_DX);
    } break;
    case 0xEE: {  // OUT DX, AL
      ins->op = OP_OUT;
      operand_set_register(&ins->dest, REG_DX);
      operand_set_register(&ins->src, REG_AL);
    } break;
    case 0xEF: {  // in DX, AX
      ins->op = OP_OUT;
      operand_set_register(&ins->dest, REG_DX);
      operand_set_register(&ins->src, REG_AX);
    } break;
    case 0XF4: ins->op = OP_HLT; break;
    case 0xF5: ins->op = OP_CMC; break;
    case 0xF6: case 0xF7: { // Grp3a and 3b (FLAG_W is set accordingly)
      data = parse_ins_grp3(ins, data);
    } break;
    case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: {
      ins->op = (ins->opcode - 0xF8) + OP_CLC;
    } break;
    case 0xFE: {
      data = parse_ins_grp4(ins, data);
    } break;
    case 0xFF: {
      data = parse_ins_grp5(ins, data);
    } break;
    default: {
      print_ins_addr(ins);
      error_exit("Illegal opcode: 0x%X\n", ins->opcode);
    }
  }
  ins->size = (uint8_t)((uint64_t)data - (uint64_t)old_data);
  return data;
}

// Prints jcc, i.e., 0x70 -- 0x7F
// These instructions are different, since their target address uses sign-extended relative offsets,
// and that the offset is after the instruction
void ins_jcc_fprint(ins_t *ins, FILE *fp) {
  assert(ins->opcode >= 0x70 && ins->opcode <= 0x7F);
  uint16_t rel_16 = (uint16_t)(int16_t)(int8_t)ins->src.rel_8;
  rel_16 += 2; // After the two-byte instruction
  // "rel" means it depends on inst's actual address (i.e., current IP)
  fprintf(fp, " rel %04X", rel_16);
  return;
}

// Only prints instruction, but not address or binary representation
void ins_fprint(ins_t *ins, FILE *fp) {
  uint8_t opcode = ins->opcode;
  uint32_t flags = ins->flags;
  // REP/REPE/REPNE
  if(opcode == 0xA4 || opcode == 0xA5 || opcode == 0xAA || opcode == 0xAB || opcode == 0xAC || opcode == 0xAD) {
    if(flags & FLAG_REP) {
      fprintf(fp, "rep ");
    }
  } else if(opcode == 0xA6 || opcode == 0xA7 || opcode == 0xAE || opcode == 0xAF) {
    if(flags & FLAG_REPE) {
      fprintf(fp, "repe ");
    } else if(flags & FLAG_REPNE) {
      fprintf(fp, "repne ");
    }
  }
  // lock prefix
  if(flags & FLAG_LOCK) {
    fprintf(fp, "lock ");
  }
  // Opcode name
  fprintf(fp, "%s", op_names[ins->op]);
  // jcc is printed differently
  if(opcode >= 0x70 && opcode <= 0x7F) {
    ins_jcc_fprint(ins, fp);
    return;
  }
  // jmp/call far using memory operand needs an extra "far"
  if((flags & FLAG_FAR) && (ins->op == OP_JMP || ins->op == OP_CALL)) {
    fprintf(fp, " far");
  }
  if(ins->dest.operand_mode != OPERAND_NONE) {
    fputc(' ', fp);
    // Certain opcodes requires operand size
    if(opcode == 0x80 || opcode == 0x81 || opcode == 0x83 || opcode == 0xC6 || opcode == 0xC7 || 
       opcode == 0xD0 || opcode == 0xD1 || opcode == 0xD2 || opcode == 0xD3) {
      if(ins->dest.operand_mode == OPERAND_MEM) {
        fprintf(fp, (flags & FLAG_W) ? "word ptr " : "byte ptr ");
      }
    }
    operand_fprint(&ins->dest, flags, fp);
  }
  if(ins->src.operand_mode != OPERAND_NONE) {
    if(ins->dest.operand_mode != OPERAND_NONE) {
      fprintf(fp, ", ");
    } else {
      fprintf(fp, " ");
    }
    // 0x83 is always 16-bit dest and 8-bit imm
    if(opcode == 0x83) {
      fprintf(fp, "byte ");
    } else if(opcode == 0x8F) {
      // POP to memory must be word ptr
      if(ins->src.operand_mode == OPERAND_MEM) {
        fprintf(fp, "word ptr ");
      }
    } else if(opcode == 0xD4 || opcode == 0xD5) {
      assert(ins->src.operand_mode == OPERAND_IMM_8);
      // AAD/AAM do not need to print 10
      if(ins->src.imm_8 == 10) {
        return;
      }
    }
    operand_fprint(&ins->src, flags, fp);
  }
  return;
}
