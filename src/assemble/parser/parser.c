#include "parser.h"
#include <stdlib.h>

#include "../../global.h"

TreeMap *first_pass(ArrayList *list) {
  if (list == NULL || list->size == 0) return NULL;
  TreeMap *found_labels = create_map(NULL, free, compare_strings_map);
  for (int i = 0; i < list->size; i++) {
    ArrayList *line = get_ArrayList_element(list, i);
    if (line == NULL || line->size == 0) return NULL;
    Token first_token = get_ArrayList_element(line, 0);
    if (first_token->type == TOKEN_TYPE_LABEL) {
      uint32_t *address = malloc(1 * sizeof(uint32_t));
      if (address == NULL) {
        IRREPARABLE_MEMORY_ERROR;
      }
      *address = i * 4;
      put_map(found_labels, first_token->labelToken.label, address);
    }
  }
  return found_labels;
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
  shift->type = shiftType.instruction;
  shift->amount = magnitude.value;
  return shift;
}

ArrayList *second_pass(ArrayList *file, TreeMap *tree) {//why return pointer?
  ArrayList *returnArray = create_ArrayList(NULL, free);
  for (int i = 0; i < file->size; i++) {
    Parser_Tree *returnTree = malloc(sizeof(Parser_Tree));
    ArrayList *line = get_ArrayList_element(file, i);
    for (int j = 1; j < line->size; j++) {
      Token currTok = get_ArrayList_element(line, j);
      if (currTok->type == TOKEN_TYPE_LABEL) {
        Token newTok = malloc(sizeof(struct Token));
        if (newTok == NULL) {
          IRREPARABLE_MEMORY_ERROR;
        }
        newTok->type = TOKEN_TYPE_IMMEDIATE;
        newTok->immediateToken.value =
            *(uint32_t *) get_map(tree, currTok->labelToken.label);
        line->elements[j] = newTok;//bad implementation:
        //TODO: create replace element function in ArrayList
      }
    }
    Token first_token = get_ArrayList_element(line, 0);
    Token second_token = get_ArrayList_element(line, 1);
    Token third_token = get_ArrayList_element(line, 2);
    Token fourth_token = get_ArrayList_element(line, 3);
    Token fifth_token = get_ArrayList_element(line, 4);
    Token sixth_token = get_ArrayList_element(line, 5);

    returnTree->instruction = first_token->instructionToken.instruction;

    if (first_token == NULL &&
        second_token == NULL &&
        third_token == NULL &&
        fourth_token == NULL &&
        fifth_token == NULL &&
        sixth_token == NULL) {//nop

      returnTree->type = Type_nop;

    } else if (first_token->type == TOKEN_TYPE_LABEL &&
               second_token == NULL &&
               third_token == NULL &&
               fourth_token == NULL &&
               fifth_token == NULL &&
               sixth_token == NULL) {//label

      break;

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
               second_token->type == TOKEN_TYPE_IMMEDIATE &&
               third_token == NULL &&
               fourth_token == NULL &&
               fifth_token == NULL &&
               sixth_token == NULL) {//.int, b, b.cond

      returnTree->type = Type_dotInt_b_bCond;
      returnTree->dotInt_b_bCond.imm = second_token->immediateToken.value;

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
               second_token->type == TOKEN_TYPE_REGISTER &&
               third_token == NULL &&
               fourth_token == NULL &&
               fifth_token == NULL &&
               sixth_token == NULL) {//br

      returnTree->type = Type_br;
      returnTree->br.R1 =
          *makeRegStruct(second_token->registerToken.register_name);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
               second_token->type == TOKEN_TYPE_REGISTER &&
               third_token->type == TOKEN_TYPE_IMMEDIATE &&
               ((fourth_token == NULL && fifth_token == NULL) ||
                  (fourth_token->type == TOKEN_TYPE_INSTRUCTION
                  && fifth_token->type == TOKEN_TYPE_IMMEDIATE)) &&
               sixth_token == NULL
               ) {//ldr, cmp, cmn, neg, negs, movk, movn, movz

      returnTree->type = Type_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit;
      returnTree->cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit.R1 =
          *makeRegStruct(second_token->registerToken.register_name);
      returnTree->cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit.imm =
          third_token->immediateToken.value;
      returnTree->cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit.shift =
          makeShiftStruct(fourth_token->instructionToken,
                          fifth_token->immediateToken);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
               second_token->type == TOKEN_TYPE_REGISTER &&
               third_token->type == TOKEN_TYPE_REGISTER &&
               ((fourth_token == NULL && fifth_token == NULL)
                  || (fourth_token->type == TOKEN_TYPE_INSTRUCTION
                  && fifth_token->type == TOKEN_TYPE_IMMEDIATE)) &&
               sixth_token == NULL) {//cmp, cmn, neg, negs, tst, mov, mvn

      returnTree->type = Type_cmp_cmn_neg_negs_REG_tst_mov_mvn;
      returnTree->cmp_cmn_neg_negs_REG_tst_mov_mvn.R1 =
          *makeRegStruct(second_token->registerToken.register_name);
      returnTree->cmp_cmn_neg_negs_REG_tst_mov_mvn.R2 =
          *makeRegStruct(third_token->registerToken.register_name);
      returnTree->cmp_cmn_neg_negs_REG_tst_mov_mvn.shift = makeShiftStruct(
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

      returnTree->type = Type_add_sub_adds_subs_IMM;
      returnTree->add_sub_adds_subs_IMM.R1 =
          *makeRegStruct(second_token->registerToken.register_name);
      returnTree->add_sub_adds_subs_IMM.R2 =
          *makeRegStruct(third_token->registerToken.register_name);
      returnTree->add_sub_adds_subs_IMM.imm =
          fourth_token->immediateToken.value;
      returnTree->add_sub_adds_subs_IMM.shift =
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

      returnTree->type = Type_add_sub_adds_subs_REG_mul_mneg_logical;
      returnTree->add_sub_adds_subs_REG_mul_mneg_logical.R1 =
          *makeRegStruct(second_token->registerToken.register_name);
      returnTree->add_sub_adds_subs_REG_mul_mneg_logical.R2 =
          *makeRegStruct(third_token->registerToken.register_name);
      returnTree->add_sub_adds_subs_REG_mul_mneg_logical.R3 =
          *makeRegStruct(fourth_token->registerToken.register_name);
      returnTree->add_sub_adds_subs_REG_mul_mneg_logical.shift =
          makeShiftStruct(fifth_token->instructionToken,
                          sixth_token->immediateToken);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
        second_token->type == TOKEN_TYPE_REGISTER &&
        third_token->type == TOKEN_TYPE_REGISTER &&
        fourth_token->type == TOKEN_TYPE_REGISTER &&
        fifth_token->type == TOKEN_TYPE_REGISTER &&
        sixth_token == NULL) {//madd, msub

      returnTree->type = Type_madd_msub;
      returnTree->madd_msub.R1 =
              *makeRegStruct(second_token->registerToken.register_name);
      returnTree->madd_msub.R2 =
              *makeRegStruct(third_token->registerToken.register_name);
      returnTree->madd_msub.R3 =
              *makeRegStruct(fourth_token->registerToken.register_name);
      returnTree->madd_msub.R4 =
              *makeRegStruct(fifth_token->registerToken.register_name);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
               second_token->type == TOKEN_TYPE_REGISTER &&
               third_token->type == TOKEN_ADDRESS_CODE &&
               fourth_token == NULL &&
               fifth_token == NULL &&
               sixth_token == NULL &&
               third_token->addressToken.t1->type == TOKEN_TYPE_REGISTER &&
               third_token->addressToken.pT2->type == TOKEN_TYPE_IMMEDIATE
               ) {//Pre-Index, Unsigned Offset

      returnTree->type = Type_ldr_str_preIndex_postIndex_unsignedOffset;
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.R1 =
              *makeRegStruct(second_token->registerToken.register_name);
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.R2 =
              *makeRegStruct(third_token->addressToken.
                             t1->registerToken.register_name);
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.imm =
              third_token->addressToken.pT2->immediateToken.value;
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.addrType =
              (third_token->addressToken.exclamation)
                  ? PRE_INDEX : UNSIGNED_OFFSET;

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
               second_token->type == TOKEN_TYPE_REGISTER &&
               third_token->type == TOKEN_ADDRESS_CODE &&
               fourth_token == NULL &&
               fifth_token == NULL &&
               sixth_token == NULL &&
               third_token->addressToken.t1->type == TOKEN_TYPE_REGISTER &&
               third_token->addressToken.pT2->type == TOKEN_TYPE_REGISTER
               ) {//Reg

      returnTree->type = Type_ldr_str_regOffset;
      returnTree->ldr_str_regOffset.R1 =
              *makeRegStruct(second_token->registerToken.register_name);
      returnTree->ldr_str_regOffset.R2 =
              *makeRegStruct(third_token->addressToken.
                             t1->registerToken.register_name);
      returnTree->ldr_str_regOffset.R3 =
              *makeRegStruct(fourth_token->addressToken.
                             pT2->registerToken.register_name);

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION &&
               second_token->type == TOKEN_TYPE_REGISTER &&
               third_token->type == TOKEN_ADDRESS_CODE &&
               fourth_token->type == TOKEN_TYPE_IMMEDIATE &&
               fifth_token == NULL &&
               sixth_token == NULL &&
               third_token->addressToken.t1->type == TOKEN_TYPE_REGISTER
               ) {//Post-Index

      returnTree->type = Type_ldr_str_preIndex_postIndex_unsignedOffset;
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.R1 =
              *makeRegStruct(second_token->registerToken.register_name);
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.R2 =
              *makeRegStruct(third_token->addressToken.
                             t1->registerToken.register_name);
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.imm =
              fourth_token->immediateToken.value;
      returnTree->ldr_str_preIndex_postIndex_unsignedOffset.addrType =
              POST_INDEX;


    } else {
      perror("invalid syntax");
      exit(EXIT_FAILURE);
    }
    add_ArrayList_element(returnArray, returnTree);
  }
  return returnArray;
}