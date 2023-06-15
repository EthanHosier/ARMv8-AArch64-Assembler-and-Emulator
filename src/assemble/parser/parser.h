#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "../lexer/lexer.h"
#include "../../ArrayList.h"
#include "../../Map.h"
#include "../register.h"


typedef struct {
    char *instruction;
    Register Rd;
    Register Rn;
    uint32_t imm;
    Shift *shift; //0 or 12
} Tree_add_sub_adds_subs_IMM;

typedef struct {
    char *instruction;
    Register Rd;
    Register Rn;
    Register Rm;
    Shift *shift;
} Tree_add_sub_adds_subs_REG;

typedef struct {
    char *instruction;
    Register R1;
    uint32_t imm;
    Shift *shift;//0 or 12
} Tree_cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM;

typedef struct {
    char *instruction;
    Register R1;
    Register Rm;
    Shift *shift;
} Tree_cmp_cmn_neg_negs_tst_mov_mvn_REG;

typedef struct {
    char *instruction;
    Register Rd;
    Register Rn;
    Register Rm;
    Shift *shift;
} Tree_logical_mul_mneg;

typedef struct {
    char *instruction;
    Register Rd;
    Register Rn;
    Register Rm;
    Register Ra;
} Tree_madd_msub;

typedef struct {
    char *instruction;
    uint32_t immediate;
} Tree_b_bCond_dotInt;

typedef struct {
    char *instruction;
    Register Rn; // MUST BE 64-BIT!!
} Tree_br;

typedef struct {
    char *instruction;
    Register Rt;
    Register Rn; // MUST BE 64-BIT!!!
    int32_t simm;
    bool isPreIndexed;
} Tree_ldr_str_pre_post;

typedef struct {
    char *instruction;
    Register Rt;
    Register Rn; // MUST BE 64-BIT!!!
    uint32_t imm;
} Tree_ldr_str_unsigned;

typedef struct {
    char *instruction;
    Register Rt;
    Register Xn; // MUST BE 64-BIT!!!
    Register Xm; // MUST BE 64-BIT!!!
} Tree_ldr_str_regOffset;

typedef struct {
    char *instruction;
    Register Rt;
    uint32_t imm;
} Tree_ldr_literal;

typedef enum {
    Type_add_sub_adds_subs_IMM,
    Type_add_sub_adds_subs_REG,
    Type_cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM ,
    Type_cmp_cmn_neg_negs_tst_mov_mvn_REG,
    Type_logical_mul_mneg,
    Type_madd_msub,
    Type_b_bCond_dotInt,
    Type_br,
    Type_ldr_str_pre_post,
    Type_ldr_str_unsigned,
    Type_ldr_str_regOffset,
    Type_nop
} tree_type;

typedef struct {
    tree_type type;
    union {
        Tree_add_sub_adds_subs_IMM add_sub_adds_subs_IMM;
        Tree_add_sub_adds_subs_REG add_sub_adds_subs_REG;
        Tree_cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM;
        Tree_cmp_cmn_neg_negs_tst_mov_mvn_REG cmp_cmn_neg_negs_tst_mov_mvn_REG;
        Tree_logical_mul_mneg logical_mul_mneg;
        Tree_madd_msub madd_msub;
        Tree_b_bCond_dotInt b_bCond_dotInt;
        Tree_br br;
        Tree_ldr_str_pre_post ldr_str_pre_post;
        Tree_ldr_str_unsigned ldr_str_unsigned;
        Tree_ldr_str_regOffset ldr_str_regOffset;
        Tree_ldr_literal ldr_literal;
    };
} Parser_Tree;

extern Map *first_pass(ArrayList *tokens);

extern Parser_Tree *second_pass(ArrayList *tokens, Map *tree);

#endif
