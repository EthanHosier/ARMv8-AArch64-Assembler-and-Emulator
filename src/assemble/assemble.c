#include "assemble.h"
#include <stdlib.h>
#include <assert.h>
#include "string.h"

int main(int argc, char **argv) {
  char line[] = "ldr x20, [x5] #8"; // post-index test
  //char line[] = "ldr x20, [x5, #8]!"; // pre-index test
  //char line[] = "ldr x20, [x5, #8]"; // unsigned offset test
  //char line[] = "ldr x20, [x5, x8]"; // reg test
  //char line1[] = "foo";
  //char line2[] = "ldr x0, foo";
  ArrayList *lines = create_ArrayList(NULL, NULL);
  add_ArrayList_element(lines, line); // read from files
  ArrayList *tokenized_lines = tokenize(lines);
  free_ArrayList(lines);
  PARSE(tokenized_lines);
  // TODO: Make map of Parser_Tree.type to function pointers that do final step
  ArrayList *binaryLines = create_ArrayList(NULL, free);
  int programCounter = 0;
  for (int i = 0; i < trees->size; i++) {
    programCounter += 4;
    Parser_Tree *tree = get_ArrayList_element(trees, i);
    tree_type type = tree->type;
    uint32_t outputVal = 0;
    if (type == Type_add_imm || type == Type_adds_imm || type == Type_sub_imm || type == Type_subs_imm) {
      buildAddAddsSubSubsImm(tree);

    } else if (type == Type_add_reg || type == Type_adds_reg || type == Type_sub_reg || type == Type_subs_reg) {
      buildAddAddsSubSubsReg(tree);

    } else if (type == Type_cmp_imm || type == Type_cmn_imm) {
      tree->type = (type == Type_cmp_imm) ? Type_subs_imm : Type_adds_imm;
      tree->R2 = tree->R1;
      tree->R1 = createZeroRegister(tree->R1->is_64_bit);
      buildAddAddsSubSubsImm(tree);

    } else if (type == Type_cmp_reg || type == Type_cmn_reg) {
      tree->type = (type == Type_cmp_reg) ? Type_subs_imm : Type_adds_imm;
      tree->R2 = tree->R1;
      tree->R1 = createZeroRegister(tree->R1->is_64_bit);
      buildAddAddsSubSubsReg(tree);

    } else if (type == Type_neg_imm || type == Type_negs_imm) {
      tree->type = (type == Type_neg_imm) ? Type_sub_imm : Type_subs_imm;
      tree->R2 = createZeroRegister(tree->R1->is_64_bit);
      buildAddAddsSubSubsImm(tree);

    } else if (type == Type_neg_reg || type == Type_negs_reg) {
      tree->type = (type == Type_neg_reg) ? Type_sub_reg : Type_subs_reg;
      tree->R2 = createZeroRegister(tree->R1->is_64_bit);
      buildAddAddsSubSubsReg(tree);

    } else if (type == Type_and || type == Type_ands || type == Type_bic || type == Type_bics || type == Type_eor || type == Type_orr || type == Type_eon || type == Type_orn) {
      buildAndAndsBicBicsEorOrrEonOrn(tree);

    } else if (type == Type_tst) {
      tree->type = Type_ands;
      tree->R2 = tree->R1;
      tree->R1 = createZeroRegister(tree->R1->is_64_bit);
      buildAndAndsBicBicsEorOrrEonOrn(tree);

    } else if (type == Type_movk || type == Type_movn || type == Type_movz) {
      buildMovkMovnMovz(tree);

    } else if (type == Type_mov) {
      tree->type = Type_orr;
      tree->R3 = tree->R2;
      tree->R2 = createZeroRegister(tree->R1->is_64_bit);
      buildAndAndsBicBicsEorOrrEonOrn(tree);

    } else if (type == Type_mvn) {
      tree->type = Type_orn;
      tree->R2 = createZeroRegister(tree->R1->is_64_bit);
      buildAndAndsBicBicsEorOrrEonOrn(tree);

    } else if (type == Type_madd || type == Type_msub) {
      buildMaddMsub(tree);

    } else if (type == Type_mul || type == Type_mneg) {
      tree->type = (type == Type_mul) ? Type_madd : Type_msub;
      tree->R4 = createZeroRegister(tree->R1->is_64_bit);
      buildMaddMsub(tree);

    } else if (type == Type_b) {
      uint32_t simm26 = (*tree->imm - programCounter) / 4;
      buildBinaryBranchUnconditional(simm26);

    } else if (type == Type_beq || type == Type_bne || type == Type_bge || type == Type_blt || type == Type_bgt || type == Type_ble || type == Type_bal) {
      uint32_t simm19 = (*tree->imm - programCounter) / 4;
      uint32_t cond = (type == Type_beq) ? 0 : (type == Type_bne) ? 1 : type - Type_bge;//enum
      buildBinaryBranchConditional(simm19, cond);

    } else if (type == Type_br) {
      uint32_t xn = tree->R1->register_number;
      buildBinaryBranchRegister(xn);

    } else {

    }
  }
  free_ArrayList(trees);
  return EXIT_SUCCESS;
}

static Register *createZeroRegister(bool is64Bit) {
  Register *reg = malloc(sizeof(Register));
  assert(reg != NULL);
  reg->is_64_bit = is64Bit;
  reg->register_number = 31;
  return reg;
}

uint32_t *buildAddAddsSubSubsImm(Parser_Tree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc =  tree->type - Type_add_imm;//enum
  uint32_t opi = 2;
  uint32_t sh = tree->shift != NULL;
  uint32_t imm12 = *(tree->imm);
  uint32_t rn = tree->R2->register_number;
  uint32_t operand = sh << 22 | imm12 << 10 | rn;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPImm(sf, opc, opi, operand, rd);
}

uint32_t *buildMovkMovnMovz(Parser_Tree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc =  (tree->type == Type_movn) ? 0 : (tree->type == Type_movz) ? 2 : 3;
  uint32_t opi = 5;
  uint32_t hw = ((tree->shift == NULL) ? 0 : tree->shift->amount) / 16;//enum;
  uint32_t imm16 = *(tree->imm);
  uint32_t operand = hw << 21 | imm16;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPImm(sf, opc, opi, operand, rd);
}


uint32_t *buildBinaryDPImm(uint32_t sf,
                          uint32_t opc,
                          uint32_t opi,
                          uint32_t operand,
                          uint32_t rd) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val =  sf << 31
      | opc << 29
      | 1 << 28
      | opi << 23
      | operand << 5
      | rd;
  return val;
}

uint32_t *buildAddAddsSubSubsReg(Parser_Tree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc =  tree->type - Type_add_reg;//enum
  uint32_t m = 0;
  uint32_t opr = 8 | ((tree->shift == NULL) ? 0 : tree->shift->type) << 1;//enum
  uint32_t rm = tree->R3->register_number;
  uint32_t operand = tree->shift->amount;
  uint32_t rn = tree->R2->register_number;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPReg(sf, opc, m, opr, rm, operand, rn, rd);
}

uint32_t *buildAndAndsBicBicsEorOrrEonOrn(Parser_Tree *tree) {
  uint32_t sf = tree->R1->is_64_bit;
  uint32_t opc_n = tree->type - Type_and;//enum
  uint32_t opc = opc_n >> 1;
  uint32_t m = 0;
  uint32_t opr = ((tree->shift == NULL) ? 0 : tree->shift->type) << 1 | (opc_n & 1);//enum
  uint32_t rm = tree->R3->register_number;
  uint32_t operand = tree->shift->amount;
  uint32_t rn = tree->R2->register_number;
  uint32_t rd = tree->R1->register_number;
  return buildBinaryDPReg(sf, opc, m, opr, rm, operand, rn, rd);
}

uint32_t *buildMaddMsub(Parser_Tree *tree) {
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

uint32_t *buildBinaryDPReg(uint32_t sf,
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
      | 5 << 26
      | opr << 21
      | rm << 16
      | operand << 10
      | rn << 5
      | rd;
  return val;
}

uint32_t *buildBinarySDT(uint32_t sf,
                        uint32_t u,
                        uint32_t l,
                        uint32_t offset,
                        uint32_t xn,
                        uint32_t rt) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val =  1 << 31
      | sf << 30
      | 7 << 27
      | u << 24
      | l << 22
      | offset << 10
      | xn << 5
      | rt;
  return val;
}

uint32_t *buildBinaryLoadLiteral(uint32_t sf, uint32_t simm19, uint32_t rt) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val =  sf << 30
      | 3 << 27
      | simm19 << 5
      | rt;
  return val;
}

uint32_t *buildBinaryBranchUnconditional(uint32_t simm26) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val =  5 << 26
          | simm26;
  return val;
}

uint32_t *buildBinaryBranchRegister(uint32_t xn) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = 0xD61F
          | xn << 5;
  return val;
}

uint32_t *buildBinaryBranchConditional(uint32_t simm19, uint32_t cond) {
  uint32_t *val = malloc(sizeof(uint32_t));
  *val = 0x15
         | simm19 << 5
         | cond;
  return val;
}

uint32_t buildNOP(void) {
  return 0xD503201F;
}
