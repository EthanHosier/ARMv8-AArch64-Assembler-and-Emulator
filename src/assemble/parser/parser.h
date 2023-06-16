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

typedef enum {
  UNCONDITIONAL,
  CONDITION_EQUALS,
  CONDITION_NOT_EQUAL,
  CONDITION_GREATER_EQUAL,
  CONDITION_LESS_THAN,
  CONDITION_GREATER_THAN,
  CONDITION_LESS_EQUAL,
  CONDITION_ALWAYS,
  DOT_INT
} b_condition_type;

typedef struct {
  uint32_t imm;
  b_condition_type condition;
} Tree_b_bCond;

typedef struct {
  uint32_t imm;
} Tree_dot_int;

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
  Type_b_cond,
  Type_br,
  Type_str, //
  Type_ldr, // for this one and the one above, seperate into addressing mode cases
  Type_load_literal,
  Type_nop,
  Type_dot_int
} tree_type;

typedef struct {
  tree_type type;
  union {
    Tree_b_bCond b_bCond;
    Tree_dot_int dot_int;
    Tree_br br;
    Tree_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit
        cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit;
    Tree_cmp_cmn_neg_negs_REG_tst_mov_mvn cmp_cmn_neg_negs_REG_tst_mov_mvn;
    Tree_add_sub_adds_subs_IMM add_sub_adds_subs_IMM;
    Tree_add_sub_adds_subs_REG_mul_mneg_logical
        add_sub_adds_subs_REG_mul_mneg_logical;
    Tree_madd_msub madd_msub;
    Tree_ldr_str_preIndex_postIndex_unsignedOffset
        ldr_str_preIndex_postIndex_unsignedOffset;
    Tree_ldr_str_regOffset ldr_str_regOffset;
  };
} Parser_Tree;

//typedef struct {
//  tree_type type;
//  Register *R1;
//  Register *R2;
//  Register *R3;
//  Register *R4;
//  uint32_t *imm;
//  Shift *shift;
//  Mem_Addr_Type_Reg_Reg_Imm *address;
//} Parser_Tree;

extern TreeMap *first_pass(ArrayList *tokens);

extern ArrayList *second_pass(ArrayList *tokens, TreeMap *tree);

#endif
