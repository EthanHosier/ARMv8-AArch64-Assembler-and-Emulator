#include "decoder.h"
#include <assert.h>
#include <stdlib.h>

static int program_counter = 0;

static int get_program_counter(void) {
  return program_counter * 4;
}

static int increment_program_counter(void) {
  return ++program_counter;
}

static Register *createZeroRegister(bool is64Bit) {
  Register *reg = malloc(sizeof(Register));
  assert(reg != NULL);
  reg->is_64_bit = is64Bit;
  reg->register_number = 31;
  return reg;
}

static uint32_t *buildBinaryDPImm(uint32_t sf,
                                  uint32_t opc,
                                  uint32_t opi,
                                  uint32_t operand,
                                  uint32_t rd) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = sf << 31
      | opc << 29
      | 1 << 28
      | opi << 23
      | operand << 5
      | rd;
  return val;
}

static uint32_t *buildBinaryDPReg(uint32_t sf,
                                  uint32_t opc,
                                  uint32_t m,
                                  uint32_t opr,
                                  uint32_t rm,
                                  uint32_t operand,
                                  uint32_t rn,
                                  uint32_t rd) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = sf << 31
      | opc << 29
      | m << 28
      | 5 << 25
      | opr << 21
      | rm << 16
      | operand << 10
      | rn << 5
      | rd;
  return val;
}

static uint32_t *buildAddAddsSubSubsReg(ParserTree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc = tree->type - Type_add_reg;//enum
  uint32_t m = 0;
  uint32_t opr = 8 | ((tree->shift == NULL) ? 0 : tree->shift->type) << 1;//enum
  uint32_t rm = tree->R3->register_number;
  uint32_t operand = (tree->shift != NULL) ? (tree->shift->amount) : 0;
  uint32_t rn = tree->R2->register_number;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPReg(sf, opc, m, opr, rm, operand, rn, rd);
}

static uint32_t *buildAndAndsBicBicsEorOrrEonOrn(ParserTree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc_n = tree->type - Type_and;//enum
  uint32_t opc = opc_n >> 1;
  uint32_t m = 0;
  uint32_t opr =
      ((tree->shift == NULL) ? 0 : tree->shift->type) << 1 | (opc_n & 1);//enum
  uint32_t rm = tree->R3->register_number;
  uint32_t operand = (tree->shift != NULL) ? (tree->shift->amount) : 0;
  uint32_t rn = tree->R2->register_number;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPReg(sf, opc, m, opr, rm, operand, rn, rd);
}

static uint32_t *buildMaddMsub(ParserTree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc = 0;
  uint32_t m = 1;
  uint32_t opr = 8;
  uint32_t rm = tree->R3->register_number;
  uint32_t x = tree->type - Type_madd;//enum
  uint32_t ra = tree->R4->register_number;
  uint32_t operand = x << 5 | ra;
  uint32_t rn = tree->R2->register_number;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPReg(sf, opc, m, opr, rm, operand, rn, rd);
}

static uint32_t *buildBinarySDT(uint32_t sf,
                                uint32_t u,
                                uint32_t l,
                                uint32_t offset,
                                uint32_t xn,
                                uint32_t rt) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = 1 << 31
      | sf << 30
      | 7 << 27
      | u << 24
      | l << 22
      | offset << 10
      | xn << 5
      | rt;
  return val;
}

static uint32_t *
buildBinaryLoadLiteral(uint32_t sf, uint32_t simm19, uint32_t rt) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = sf << 30
      | 3 << 27
      | simm19 << 5
      | rt;
  return val;
}

static uint32_t *buildBinaryBranchUnconditional(uint32_t simm26) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = 5 << 26
      | simm26;
  return val;
}

static uint32_t *buildBinaryBranchRegister(uint32_t xn) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = 0xD61F << 16
      | xn << 5;
  return val;
}

static uint32_t *buildBinaryBranchConditional(uint32_t simm19, uint32_t cond) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = 84 << 24
      | simm19 << 5
      | 0 << 4
      | cond;
  return val;
}

static uint32_t *buildAddAddsSubSubsImm(ParserTree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc = tree->type - Type_add_imm;//enum
  uint32_t opi = 2;
  uint32_t sh = tree->shift != NULL && tree->shift->amount != 0;
  uint32_t imm12 = *(tree->imm);
  uint32_t rn = tree->R2->register_number;
  uint32_t operand = sh << (22 - 5) | imm12 << (10 - 5) | rn;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPImm(sf, opc, opi, operand, rd);
}

static uint32_t *cmp_cmn_imm(ParserTree *tree) {
  tree->type = (tree->type == Type_cmp_imm) ? Type_subs_imm : Type_adds_imm;
  tree->R2 = tree->R1;
  tree->R1 = createZeroRegister(tree->R1->is_64_bit);
  return buildAddAddsSubSubsImm(tree);
}

static uint32_t *cmp_cmn_reg(ParserTree *tree) {
  tree->type = (tree->type == Type_cmp_reg) ? Type_subs_reg : Type_adds_reg;
  Register *temp = tree->R2;
  tree->R2 = tree->R1;
  tree->R3 = temp;
  tree->R1 = createZeroRegister(tree->R1->is_64_bit);
  return buildAddAddsSubSubsReg(tree);
}

static uint32_t *neg_negs_imm(ParserTree *tree) {
  tree->type = (tree->type == Type_neg_imm) ? Type_sub_imm : Type_subs_imm;
  tree->R2 = createZeroRegister(tree->R1->is_64_bit);
  return buildAddAddsSubSubsImm(tree);
}

static uint32_t *neg_negs_reg(ParserTree *tree) {
  tree->type = (tree->type == Type_neg_reg) ? Type_sub_reg : Type_subs_reg;
  tree->R2 = createZeroRegister(tree->R1->is_64_bit);
  return buildAddAddsSubSubsReg(tree);
}

static uint32_t *tst(ParserTree *tree) {
  tree->type = Type_ands;
  tree->R3 = tree->R2;
  tree->R2 = tree->R1;
  tree->R1 = createZeroRegister(tree->R1->is_64_bit);
  return buildAndAndsBicBicsEorOrrEonOrn(tree);
}

static uint32_t *mov(ParserTree *tree) {
  tree->type = Type_orr;
  tree->R3 = tree->R2;
  tree->R2 = createZeroRegister(tree->R1->is_64_bit);
  return buildAndAndsBicBicsEorOrrEonOrn(tree);
}

static uint32_t *mvn(ParserTree *tree) {
  tree->type = Type_orn;
  tree->R2 = createZeroRegister(tree->R1->is_64_bit);
  return buildAndAndsBicBicsEorOrrEonOrn(tree);
}

static uint32_t *mul_mneg(ParserTree *tree) {
  tree->type = (tree->type == Type_mul) ? Type_madd : Type_msub;
  tree->R4 = createZeroRegister(tree->R1->is_64_bit);
  return buildMaddMsub(tree);
}

static uint32_t *b(ParserTree *tree) {
  uint32_t simm26 = (*tree->imm - get_program_counter()) / 4;
  return buildBinaryBranchUnconditional(simm26);
}

static uint32_t *b_cond(ParserTree *tree) {
  uint32_t simm19 = (*tree->imm - get_program_counter()) / 4;
  uint32_t cond =
      (tree->type == Type_beq) ? 0 : (tree->type == Type_bne) ? 1 : tree->type
          - Type_bge + 10;//enum
  return buildBinaryBranchConditional(simm19, cond);
}

static uint32_t *br(ParserTree *tree) {
  uint32_t xn = tree->R1->register_number;
  return buildBinaryBranchRegister(xn);
}

static uint32_t *sdt(ParserTree *tree) {
  if (tree->type == Type_ldr_unsigned || tree->type == Type_str_unsigned) {
    if (tree->R1->is_64_bit) *tree->imm = *tree->imm / 8;
    else *tree->imm = *tree->imm / 4;
  }
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t
      u = tree->type == Type_ldr_unsigned || tree->type == Type_str_unsigned;
  uint32_t l = tree->type == Type_ldr_unsigned || tree->type == Type_ldr_pre
      || tree->type == Type_ldr_post || tree->type == Type_ldr_reg;
  uint32_t offset = 0;
  if (tree->type == Type_ldr_unsigned || tree->type == Type_str_unsigned) {
    offset = *tree->imm;
  } else if (tree->type == Type_ldr_reg || tree->type == Type_str_reg) {
    offset = 1 << 11 | tree->R3->register_number << 6 | 13 << 1;
  } else {//pre/post
    offset = (0x1FF & *tree->imm) << 2 | (tree->type == Type_ldr_pre || tree->type == Type_str_pre) << 1 | 1;
  }
  uint32_t xn = tree->R2->register_number;
  uint32_t rt = tree->R1->register_number;
  return buildBinarySDT(sf, u, l, offset, xn, rt);
}

static uint32_t *load_literal(ParserTree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t simm19 = 0x7FFFF & (((int64_t) *tree->imm - (int64_t) get_program_counter()) / 4);
  uint32_t rt = tree->R1->register_number;
  return buildBinaryLoadLiteral(sf, simm19, rt);
}

static uint32_t *dot_int(ParserTree *tree) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = *tree->imm;
  return val;
}

static uint32_t *nop(ParserTree *tree) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = 0xD503201F;
  return val;
}

static uint32_t *buildMovkMovnMovz(ParserTree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t
      opc = (tree->type == Type_movn) ? 0 : (tree->type == Type_movz) ? 2 : 3;
  uint32_t opi = 5;
  uint32_t hw = ((tree->shift == NULL) ? 0 : tree->shift->amount) / 16;//enum;
  uint32_t imm16 = *(tree->imm);
  uint32_t operand = hw << 16 | imm16;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPImm(sf, opc, opi, operand, rd);
}

uint32_t *decoder(ParserTree *tree) {
  uint32_t *outputVal;
  TreeMap *map = create_map(free, NULL, compare_ints_map);
  put_map_int_key(map, Type_add_imm, buildAddAddsSubSubsImm);
  put_map_int_key(map, Type_adds_imm, buildAddAddsSubSubsImm);
  put_map_int_key(map, Type_sub_imm, buildAddAddsSubSubsImm);
  put_map_int_key(map, Type_subs_imm, buildAddAddsSubSubsImm);

  put_map_int_key(map, Type_add_reg, buildAddAddsSubSubsReg);
  put_map_int_key(map, Type_adds_reg, buildAddAddsSubSubsReg);
  put_map_int_key(map, Type_sub_reg, buildAddAddsSubSubsReg);
  put_map_int_key(map, Type_subs_reg, buildAddAddsSubSubsReg);

  put_map_int_key(map, Type_cmp_imm, cmp_cmn_imm);
  put_map_int_key(map, Type_cmn_imm, cmp_cmn_imm);

  put_map_int_key(map, Type_cmp_reg, cmp_cmn_reg);
  put_map_int_key(map, Type_cmn_reg, cmp_cmn_reg);

  put_map_int_key(map, Type_neg_imm, neg_negs_imm);
  put_map_int_key(map, Type_negs_imm, neg_negs_imm);

  put_map_int_key(map, Type_neg_reg, neg_negs_reg);
  put_map_int_key(map, Type_negs_reg, neg_negs_reg);

  put_map_int_key(map, Type_and, buildAndAndsBicBicsEorOrrEonOrn);
  put_map_int_key(map, Type_ands, buildAndAndsBicBicsEorOrrEonOrn);
  put_map_int_key(map, Type_bic, buildAndAndsBicBicsEorOrrEonOrn);
  put_map_int_key(map, Type_bics, buildAndAndsBicBicsEorOrrEonOrn);
  put_map_int_key(map, Type_eor, buildAndAndsBicBicsEorOrrEonOrn);
  put_map_int_key(map, Type_orr, buildAndAndsBicBicsEorOrrEonOrn);
  put_map_int_key(map, Type_eon, buildAndAndsBicBicsEorOrrEonOrn);
  put_map_int_key(map, Type_orn, buildAndAndsBicBicsEorOrrEonOrn);

  put_map_int_key(map, Type_tst, tst);

  put_map_int_key(map, Type_movk, buildMovkMovnMovz);
  put_map_int_key(map, Type_movn, buildMovkMovnMovz);
  put_map_int_key(map, Type_movz, buildMovkMovnMovz);

  put_map_int_key(map, Type_mov, mov);

  put_map_int_key(map, Type_mvn, mvn);

  put_map_int_key(map, Type_madd, buildMaddMsub);
  put_map_int_key(map, Type_msub, buildMaddMsub);

  put_map_int_key(map, Type_mul, mul_mneg);
  put_map_int_key(map, Type_mneg, mul_mneg);

  put_map_int_key(map, Type_b, b);

  put_map_int_key(map, Type_beq, b_cond);
  put_map_int_key(map, Type_bne, b_cond);
  put_map_int_key(map, Type_bge, b_cond);
  put_map_int_key(map, Type_blt, b_cond);
  put_map_int_key(map, Type_bgt, b_cond);
  put_map_int_key(map, Type_ble, b_cond);
  put_map_int_key(map, Type_bal, b_cond);

  put_map_int_key(map, Type_br, br);

  put_map_int_key(map, Type_ldr_unsigned, sdt);
  put_map_int_key(map, Type_str_unsigned, sdt);
  put_map_int_key(map, Type_ldr_pre, sdt);
  put_map_int_key(map, Type_str_pre, sdt);
  put_map_int_key(map, Type_ldr_post, sdt);
  put_map_int_key(map, Type_str_post, sdt);
  put_map_int_key(map, Type_ldr_reg, sdt);
  put_map_int_key(map, Type_str_reg, sdt);

  put_map_int_key(map, Type_ldr_literal, load_literal);

  put_map_int_key(map, Type_dot_int, dot_int);

  put_map_int_key(map, Type_nop, nop);
  decode_function function = get_map_int_key(map, tree->type);
  outputVal = (function)(tree);
  free_map(map);
  increment_program_counter();
  return outputVal;
}
