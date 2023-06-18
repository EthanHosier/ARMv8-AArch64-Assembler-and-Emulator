#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../../global.h"

#define FREE_REGISTER(reg) if((reg)!=NULL) free(reg)

TreeMap *first_pass(ArrayList *list) {
  if (list == NULL || list->size == 0) return NULL;
  TreeMap *found_labels = create_map(NULL, free, compare_strings_map);
  int instruction_number = 0;
  for (int i = 0; i < list->size; i++) {
    ArrayList *line = get_ArrayList_element(list, i);
    if (line == NULL) return NULL;
    if (line->size == 0) continue;
    Token first_token = get_ArrayList_element(line, 0);
    if (first_token->type == TOKEN_TYPE_LABEL) {
      size_t length = strlen(first_token->labelToken.label);
      first_token->labelToken.label[length - 1] = '\0';
      put_map_int(found_labels,
                  first_token->labelToken.label,
                  instruction_number * 4);
      continue;
    }
    instruction_number++;
  }
  return found_labels;
}

void free_parser_tree(void *input) {
  Parser_Tree *tree = (Parser_Tree *) input;
  if (tree == NULL) return;
  FREE_REGISTER(tree->R1);
  FREE_REGISTER(tree->R2);
  FREE_REGISTER(tree->R3);
  FREE_REGISTER(tree->R4);
  if (tree->imm != NULL) free(tree->imm);
  if (tree->shift != NULL) free(tree->shift);
  free(tree);
}

static uint32_t *make_new_int(uint32_t input) {
  uint32_t *new = malloc(sizeof(uint32_t));
  *new = input;
  return new;
}

static void b_cond(Parser_Tree *tree, Token token) {
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  put_map_int(map, "b.eq", Type_beq);
  put_map_int(map, "b.ne", Type_bne);
  put_map_int(map, "b.ge", Type_bge);
  put_map_int(map, "b.lt", Type_blt);
  put_map_int(map, "b.gt", Type_bgt);
  put_map_int(map, "b.le", Type_ble);
  put_map_int(map, "b.al", Type_bal);
  tree->type = get_map_int(map, token->instructionToken.instruction);
  free_map(map);
}

static void discriminator1(Parser_Tree *tree, Token token) {
  // ldr, cmp, cmn, neg, negs, movk, movn, movz
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  put_map_int(map, "ldr", Type_ldr_literal);
  put_map_int(map, "cmp", Type_cmp_imm);
  put_map_int(map, "cmn", Type_cmn_imm);
  put_map_int(map, "neg", Type_neg_imm);
  put_map_int(map, "negs", Type_negs_imm);
  put_map_int(map, "movk", Type_movk);
  put_map_int(map, "movn", Type_movn);
  put_map_int(map, "movz", Type_movz);
  tree->type = get_map_int(map, token->instructionToken.instruction);
  free_map(map);
}

static void discriminator2(Parser_Tree *tree, Token token) {
  // Type_cmp_cmn_neg_negs_REG_tst_mov_mvn;
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  put_map_int(map, "cmp", Type_cmp_reg);
  put_map_int(map, "cmn", Type_cmn_reg);
  put_map_int(map, "neg", Type_neg_reg);
  put_map_int(map, "negs", Type_negs_reg);
  put_map_int(map, "tst", Type_tst);
  put_map_int(map, "mov", Type_mov);
  put_map_int(map, "mvn", Type_mvn);
  tree->type = get_map_int(map, token->instructionToken.instruction);
  free_map(map);
}

static void discriminator3(Parser_Tree *tree, Token token) {
  // Type_add_sub_adds_subs_IMM
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  put_map_int(map, "add", Type_add_imm);
  put_map_int(map, "sub", Type_sub_imm);
  put_map_int(map, "adds", Type_adds_imm);
  put_map_int(map, "subs", Type_subs_imm);
  tree->type = get_map_int(map, token->instructionToken.instruction);
  free_map(map);
}

static void discriminator4(Parser_Tree *tree, Token token) {
  // Type_add_sub_adds_subs_REG_mul_mneg_logical
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  put_map_int(map, "add", Type_add_reg);
  put_map_int(map, "sub", Type_sub_reg);
  put_map_int(map, "adds", Type_adds_reg);
  put_map_int(map, "subs", Type_subs_reg);
  put_map_int(map, "mul", Type_mul);
  put_map_int(map, "mneg", Type_mneg);
  put_map_int(map, "and", Type_and);
  put_map_int(map, "bic", Type_bic);
  put_map_int(map, "orr", Type_orr);
  put_map_int(map, "orn", Type_orn);
  put_map_int(map, "eor", Type_eor);
  put_map_int(map, "eon", Type_eon);
  put_map_int(map, "ands", Type_ands);
  put_map_int(map, "bics", Type_bics);
  tree->type = get_map_int(map, token->instructionToken.instruction);
  free_map(map);
}

static void discriminator5(Parser_Tree *tree, Token token) {
  //Type_madd_msub
  if (strcmp(token->instructionToken.instruction, "madd") == 0)
    tree->type = Type_madd;
  else tree->type = Type_msub;
}

static void discriminator6(Parser_Tree *tree, Token token) {
  // Type_ldr_str_preIndex
  if (strcmp("ldr", token->instructionToken.instruction) == 0)
    tree->type = Type_ldr_pre;
  else tree->type = Type_str_pre;
}

static void discriminator7(Parser_Tree *tree, Token token) {
  // Type_ldr_str_postIndex
  if (strcmp("ldr", token->instructionToken.instruction) == 0)
    tree->type = Type_ldr_post;
  else tree->type = Type_str_post;
}

static void discriminator8(Parser_Tree *tree, Token token) {
  // Type_ldr_str_unsignedOffset
  if (strcmp("ldr", token->instructionToken.instruction) == 0)
    tree->type = Type_ldr_unsigned;
  else tree->type = Type_str_unsigned;
}

static void discriminator9(Parser_Tree *tree, Token token) {
  // Type_ldr_str_reg
  if (strcmp("ldr", token->instructionToken.instruction) == 0)
    tree->type = Type_ldr_reg;
  else tree->type = Type_str_reg;
}

static shift_type discriminator10(char *shift) {
  // Type_lsl_lsr_asr_ror
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  put_map_int(map, "lsl", Type_lsl);
  put_map_int(map, "lsr", Type_lsr);
  put_map_int(map, "asr", Type_asr);
  put_map_int(map, "ror", Type_ror);
  shift_type type = get_map_int(map, shift);
  free_map(map);
  return type;
}

static Register *makeRegStruct(char *regString) {
  Register *reg = malloc(sizeof(Register));
  if (reg == NULL) {
    IRREPARABLE_MEMORY_ERROR;
  }
  reg->is_64_bit = regString[0] == 'x';
  reg->register_number = (int) strtol(++regString, NULL, 10);
  return reg;
}

static Shift *
makeShiftStruct(InstructionToken shiftType, ImmediateToken magnitude) {
  Shift *shift = malloc(sizeof(Shift));
  if (shift == NULL) {
    IRREPARABLE_MEMORY_ERROR;
  }
  shift->type = discriminator10(shiftType.instruction);
  shift->amount = magnitude.value;
  return shift;
}

static Parser_Tree *make_parser_tree(void) {
  Parser_Tree *returnTree = malloc(sizeof(Parser_Tree));
  assert(returnTree != NULL);
  returnTree->R1 = NULL;
  returnTree->R2 = NULL;
  returnTree->R3 = NULL;
  returnTree->R4 = NULL;
  returnTree->shift = NULL;
  returnTree->imm = NULL;
  return returnTree;
}

ArrayList *second_pass(ArrayList *file, TreeMap *tree) {//why return pointer?
  ArrayList *returnArray = create_ArrayList(NULL, free_parser_tree);
  for (int i = 0; i < file->size; i++) {
    ArrayList *line = get_ArrayList_element(file, i);
    Token first_token = get_ArrayList_element(line, 0);
    Token second_token = get_ArrayList_element(line, 1);
    Token third_token = get_ArrayList_element(line, 2);
    Token fourth_token = get_ArrayList_element(line, 3);
    Token fifth_token = get_ArrayList_element(line, 4);
    Token sixth_token = get_ArrayList_element(line, 5);
    if (first_token == NULL) continue;
    if (first_token->type == TOKEN_TYPE_LABEL &&
        second_token == NULL &&
        third_token == NULL &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL) {//label
      continue;
    }
    Parser_Tree *returnTree = make_parser_tree();

    for (int j = 1; j < line->size; j++) {
      Token currTok = get_ArrayList_element(line, j);
      if (currTok->type == TOKEN_TYPE_LABEL) {
        char *old_label_string = strdup(currTok->labelToken.label);
        free(currTok->labelToken.label);
        currTok->type = TOKEN_TYPE_IMMEDIATE;
        currTok->immediateToken.value =
            get_map_int(tree, old_label_string);
        free(old_label_string);
      }
    }
    if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        strcmp(first_token->instructionToken.instruction, "nop") == 0 &&
        second_token == NULL &&
        third_token == NULL &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL) {//nop

      returnTree->type = Type_nop;

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_IMMEDIATE &&
        third_token == NULL &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL) {//b, b.cond

      if (strcmp(first_token->instructionToken.instruction, "b") == 0)
        returnTree->type = Type_b;
      else b_cond(returnTree, first_token);
      returnTree->imm = make_new_int(second_token->immediateToken.value);
    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token == NULL &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL) {//br

      returnTree->type = Type_br;
      returnTree->R1 = makeRegStruct(second_token->registerToken.register_name);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_TYPE_IMMEDIATE &&
        ((fourth_token == NULL && fifth_token == NULL) ||
            (fourth_token->type == TOKEN_TYPE_INSTRUCTION
                && fifth_token->type == TOKEN_TYPE_IMMEDIATE)) &&
        sixth_token == NULL
        ) {//ldr, cmp, cmn, neg, negs, movk, movn, movz

      discriminator1(returnTree, first_token);
      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->imm = make_new_int(third_token->immediateToken.value);
      if (fourth_token != NULL && fifth_token != NULL)
        returnTree->shift =
            makeShiftStruct(fourth_token->instructionToken,
                            fifth_token->immediateToken);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_TYPE_REGISTER &&
        ((fourth_token == NULL && fifth_token == NULL)
            || (fourth_token->type == TOKEN_TYPE_INSTRUCTION
                && fifth_token->type == TOKEN_TYPE_IMMEDIATE)) &&
        sixth_token == NULL) {//cmp, cmn, neg, negs, tst, mov, mvn

      discriminator2(returnTree, first_token);
      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->R2 = makeRegStruct(third_token->registerToken.register_name);
      if (fourth_token != NULL && fifth_token != NULL)
        returnTree->shift = makeShiftStruct(
            fourth_token->instructionToken,
            fifth_token->immediateToken);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_TYPE_REGISTER &&
        fourth_token->type == TOKEN_TYPE_IMMEDIATE &&
        ((fifth_token == NULL && sixth_token == NULL)
            || (fifth_token->type == TOKEN_TYPE_INSTRUCTION
                && sixth_token->type == TOKEN_TYPE_IMMEDIATE))
        ) {//add, adds, sub, subs

      discriminator3(returnTree, first_token);
      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->R2 = makeRegStruct(third_token->registerToken.register_name);
      returnTree->imm = make_new_int(fourth_token->immediateToken.value);
      if (fifth_token != NULL && sixth_token != NULL)
        returnTree->shift =
            makeShiftStruct(fifth_token->instructionToken,
                            sixth_token->immediateToken);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_TYPE_REGISTER &&
        fourth_token->type == TOKEN_TYPE_REGISTER &&
        ((fifth_token == NULL && sixth_token == NULL)
            || (fifth_token->type == TOKEN_TYPE_INSTRUCTION
                && sixth_token->type == TOKEN_TYPE_IMMEDIATE))
        ) {//add, adds, sub, subs, mul, mneg, logic

      discriminator4(returnTree, first_token);
      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->R2 =
          makeRegStruct(third_token->registerToken.register_name);
      returnTree->R3 =
          makeRegStruct(fourth_token->registerToken.register_name);
      if (fifth_token != NULL && sixth_token != NULL)
        returnTree->shift =
            makeShiftStruct(fifth_token->instructionToken,
                            sixth_token->immediateToken);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_TYPE_REGISTER &&
        fourth_token->type == TOKEN_TYPE_REGISTER &&
        fifth_token->type == TOKEN_TYPE_REGISTER &&
        sixth_token == NULL) {//madd, msub

      discriminator5(returnTree, first_token);
      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->R2 =
          makeRegStruct(third_token->registerToken.register_name);
      returnTree->R3 =
          makeRegStruct(fourth_token->registerToken.register_name);
      returnTree->R4 =
          makeRegStruct(fifth_token->registerToken.register_name);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_ADDRESS_CODE &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL &&
        third_token->addressToken.t1->type == TOKEN_TYPE_REGISTER &&
        third_token->addressToken.pT2->type == TOKEN_TYPE_IMMEDIATE
        ) {//Pre-Index, Unsigned Offset
      if (third_token->addressToken.exclamation)
        discriminator6(returnTree,
                       first_token);
      else discriminator8(returnTree, first_token);

      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->R2 =
          makeRegStruct(third_token->addressToken.
              t1->registerToken.register_name);
      returnTree->imm =
          make_new_int(third_token->addressToken.pT2->immediateToken.value);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_ADDRESS_CODE &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL &&
        third_token->addressToken.t1->type == TOKEN_TYPE_REGISTER &&
        third_token->addressToken.pT2->type == TOKEN_TYPE_REGISTER
        ) {//Reg

      discriminator9(returnTree, first_token);
      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->R2 =
          makeRegStruct(third_token->addressToken.
              t1->registerToken.register_name);
      returnTree->R3 =
          makeRegStruct(third_token->addressToken.
              pT2->registerToken.register_name);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_ADDRESS_CODE &&
        fourth_token->type == TOKEN_TYPE_IMMEDIATE &&
        fifth_token == NULL &&
        sixth_token == NULL &&
        third_token->addressToken.t1->type == TOKEN_TYPE_REGISTER
        ) {//Post-Index

      discriminator7(returnTree, first_token);
      returnTree->R1 =
          makeRegStruct(second_token->registerToken.register_name);
      returnTree->R2 =
          makeRegStruct(third_token->addressToken.
              t1->registerToken.register_name);
      returnTree->imm = make_new_int(fourth_token->immediateToken.value);

    } else if (first_token->type == TOKEN_TYPE_DOT_INT &&
        second_token->type == TOKEN_TYPE_IMMEDIATE &&
        third_token == NULL &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL)
      returnTree->type = Type_dot_int;
    else {
      perror("invalid syntax");
      exit(EXIT_FAILURE);
    }
    add_ArrayList_element(returnArray, returnTree);
  }
  return returnArray;
}