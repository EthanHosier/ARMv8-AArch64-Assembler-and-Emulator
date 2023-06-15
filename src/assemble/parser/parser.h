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
    Register_or_immediate Rm_or_immediate;
    Shift *shift;
} Tree_add_sub; // also includes flag setting instructions

typedef struct {
    char *instruction;
    Register R1;
    Register_or_immediate Rm_or_immediate;
    Shift *shift;
} Tree_cmp_cmn_neg_negs;

typedef struct {
    char *instruction;
    Register Rd;
    Register Rn;
    Register Rm;
    Shift shift;
} Tree_logical_instructions;

typedef struct {
    Register R1;
    Register_or_immediate Rm_or_immediate;
    Shift *shift;
} Tree_test_movk_movn_movz_mvn;

typedef struct {
    Register Rd;
    Register Rn;
} Tree_mov;

typedef struct {
    Register Rd;
    Register Rn;
    Register Rm;
    Register Ra;
} Tree_madd_msub;

typedef struct {
    Register Rd;
    Register Rn;
    Register Rm;
} Tree_mul_mneg;

typedef struct {
    uint32_t imm;
} Tree_b_and_b_cond;

typedef struct {
    Register Rn; // MUST BE 64-BIT!!
} Tree_br;

typedef struct {
    Register Rt;
    Register Rn; // MUST BE 64-BIT!!!
    Register_or_immediate R3;
} Tree_single_data_transfer;

typedef struct {
    Register Rt;
    uint32_t immediate;
} Tree_load_literal;

typedef struct {
    uint32_t immediate;
} Tree_dot_int;

typedef enum {
    Type_add_sub,
    Type_cmp_cmn_neg_negs,
    Type_logical_instructions,
    Type_test_movk_movn_movz_mvn,
    Type_mov,
    Type_madd_msub,
    Type_mul_mneg,
    Type_b_and_b_cond,
    Type_br,
    Type_single_data_transfer,
    Type_load_literal,
    Type_dot_int
} tree_type;

typedef struct {
    tree_type type;
    union {
        Tree_add_sub add_sub;
        Tree_cmp_cmn_neg_negs cmp_cmn_neg_negs;
        Tree_logical_instructions logical_instructions;
        Tree_test_movk_movn_movz_mvn test_movk_movn_movz_mvn;
        Tree_mov mov;
        Tree_madd_msub madd_msub;
        Tree_mul_mneg mul_mneg;
        Tree_b_and_b_cond b_and_b_cond;
        Tree_br br;
        Tree_single_data_transfer single_data_transfer;
        Tree_load_literal load_literal;
        Tree_dot_int dot_int;
    };
} Parser_Tree;

extern Map *first_pass(ArrayList *tokens);

extern Parser_Tree *second_pass(ArrayList *tokens, Map *tree);

#endif
