#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "../lexer/lexer.h"
#include "../../ArrayList.h"
#include "../../TreeMap.h"
#include "../register.h"

typedef enum {
    POST_INDEX,
    PRE_INDEX,
    UNSIGNED_OFFSET
} Mem_Addr_Type_Reg_Reg_Imm;

typedef struct {
  uint32_t imm;
} Tree_dotInt_b_bCond;

typedef struct {
  Register R1;
} Tree_br;

typedef struct {
  Register R1;
  uint32_t imm;
  Shift *shift;
} Tree_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit;

typedef struct {
  Register R1;
  Register R2;
  Shift *shift;
} Tree_cmp_cmn_neg_negs_REG_tst_mov_mvn;

typedef struct {
  Register R1;
  Register R2;
  uint32_t imm;
  Shift *shift;
} Tree_add_sub_adds_subs_IMM;

typedef struct {
  Register R1;
  Register R2;
  Register R3;
  Shift *shift;
} Tree_add_sub_adds_subs_REG_mul_mneg_logical;

typedef struct {
  Register R1;
  Register R2;
  Register R3;
  Register R4;
} Tree_madd_msub;

typedef struct {
  Register R1;
  Register R2;//64Bit
  uint32_t imm;
  Mem_Addr_Type_Reg_Reg_Imm addrType;
} Tree_ldr_str_preIndex_postIndex_unsignedOffset;

typedef struct {
  Register R1;
  Register R2;//64Bit
  Register R3;//64Bit
} Tree_ldr_str_regOffset;

typedef enum {
  Type_dotInt_b_bCond,
  Type_br,
  Type_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit,
  Type_cmp_cmn_neg_negs_REG_tst_mov_mvn,
  Type_add_sub_adds_subs_IMM,
  Type_add_sub_adds_subs_REG_mul_mneg_logical,
  Type_madd_msub,
  Type_ldr_str_preIndex_postIndex_unsignedOffset,
  Type_ldr_str_regOffset,
  Type_nop
} tree_type;

typedef struct {
    char *instruction;
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
    Tree_ldr_str_preIndex_postIndex_unsignedOffset ldr_str_preIndex_postIndex_unsignedOffset;
    Tree_ldr_str_regOffset ldr_str_regOffset;
  };
} Parser_Tree;

extern TreeMap *first_pass(ArrayList *tokens);

extern ArrayList *second_pass(ArrayList *tokens, TreeMap *tree);

#endif
