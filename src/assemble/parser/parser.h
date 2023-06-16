#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "../lexer/lexer.h"
#include "../../ArrayList.h"
#include "../../TreeMap.h"
#include "../register.h"

typedef struct {
  char *instruction;
  uint32_t imm;
} Tree_dotInt_b_bCond;

typedef struct {
  char *instruction;
  Register R1;
} Tree_br;

typedef struct {
  char *instruction;
  Register R1;
  uint32_t imm;
  Shift *shift;
} Tree_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit;

typedef struct {
  char *instruction;
  Register R1;
  Register R2;
  Shift *shift;
} Tree_cmp_cmn_neg_negs_REG_tst_mov_mvn;

typedef struct {
  char *instruction;
  Register R1;
  Register R2;
  uint32_t imm;
  Shift *shift;
} Tree_add_sub_adds_subs_IMM;

typedef struct {
  char *instruction;
  Register R1;
  Register R2;
  Register R3;
  Shift *shift;
} Tree_add_sub_adds_subs_REG_mul_mneg_logical;

typedef struct {
  char *instruction;
  Register R1;
  Register R2;
  Register R3;
  Register R4;
} Tree_madd_msub;

typedef struct {
  char *instruction;
  Register R1;
  Register W2;//64Bit
  int32_t simm;
  bool isPreIndexed;
} Tree_ldr_str_preIndex_postIndex;

typedef struct {
  char *instruction;
  Register R1;
  Register W2;//64Bit
  uint32_t imm;
} Tree_ldr_str_unsigned;

typedef struct {
  char *instruction;
  Register R1;
  Register W2;//64Bit
  Register W3;//64Bit
} Tree_ldr_str_regOffset;

typedef enum {
  Type_dotInt_b_bCond,
  Type_br,
  Type_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit,
  Type_cmp_cmn_neg_negs_REG_tst_mov_mvn,
  Type_add_sub_adds_subs_IMM,
  Type_add_sub_adds_subs_REG_mul_mneg_logical,
  Type_madd_msub,
  Type_ldr_str_preIndex_postIndex,
  Type_ldr_str_unsigned,
  Type_ldr_str_regOffset,
  Type_nop
} tree_type;

typedef struct {
  tree_type type;
  union {
    Tree_dotInt_b_bCond dotInt_b_bCond;
    Tree_br br;
    Tree_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit
        cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit;
    Tree_cmp_cmn_neg_negs_REG_tst_mov_mvn cmp_cmn_neg_negs_REG_tst_mov_mvn;
    Tree_add_sub_adds_subs_IMM add_sub_adds_subs_IMM;
    Tree_add_sub_adds_subs_REG_mul_mneg_logical
        add_sub_adds_subs_REG_mul_mneg_logical;
    Tree_madd_msub madd_msub;
    Tree_ldr_str_preIndex_postIndex ldr_str_preIndex_postIndex;
    Tree_ldr_str_unsigned ldr_str_unsigned;
    Tree_ldr_str_regOffset ldr_str_regOffset;
  };
} Parser_Tree;

extern TreeMap *first_pass(ArrayList *tokens);

extern ArrayList *second_pass(ArrayList *tokens, TreeMap *tree);

#endif
