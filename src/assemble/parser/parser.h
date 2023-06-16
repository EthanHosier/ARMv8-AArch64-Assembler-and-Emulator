#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "../lexer/lexer.h"
#include "../../ArrayList.h"
#include "../../TreeMap.h"
#include "../register.h"


typedef enum {
  Type_add_imm,
  Type_adds_imm,
  Type_sub_imm,
  Type_subs_imm,
  Type_add_reg,
  Type_adds_reg,
  Type_sub_reg,
  Type_subs_reg,
  Type_cmp_imm,
  Type_cmn_imm,
  Type_cmp_reg,
  Type_cmn_reg,
  Type_neg_imm,
  Type_negs_imm,
  Type_neg_reg,
  Type_negs_reg,
  Type_and,
  Type_ands,
  Type_bic,
  Type_bics,
  Type_eor,
  Type_orr,
  Type_eon,
  Type_orn,
  Type_tst,
  Type_movk,
  Type_movn,
  Type_movz,
  Type_mov,
  Type_mvn,
  Type_madd,
  Type_msub,
  Type_mul,
  Type_mneg,
  Type_b,
  Type_beq,
  Type_bne,
  Type_bge,
  Type_blt,
  Type_bgt,
  Type_ble,
  Type_bal,
  Type_br,
  Type_str_post,
  Type_ldr_post,
  Type_str_pre,
  Type_ldr_pre,
  Type_str_unsigned,
  Type_ldr_unsigned,
  Type_str_reg,
  Type_ldr_reg,
  Type_ldr_literal,
  Type_nop,
  Type_dot_int
} tree_type;

typedef struct {
  tree_type type;
  Register *R1;
  Register *R2;
  Register *R3;
  Register *R4;
  uint32_t *imm;
  Shift *shift;
} Parser_Tree;

extern TreeMap *first_pass(ArrayList *tokens);

extern ArrayList *second_pass(ArrayList *tokens, TreeMap *tree);

extern void free_parser_tree(void *parser_tree);

#endif
