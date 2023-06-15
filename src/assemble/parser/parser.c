#include "parser.h"
#include <stdlib.h>

Map *first_pass(ArrayList *list) {
  if (list == NULL || list->size == 0) return NULL;
  Map *found_labels = create_map();
  for (int i = 0; i < list->size; i++) {
    ArrayList *line = get_ArrayList_element(list, i);
    if (line == NULL || line->size == 0) return NULL;
    Token first_token = get_ArrayList_element(line, 0);
    if (first_token->type == TOKEN_TYPE_LABEL)
      put_map(found_labels, first_token->labelToken.label, i * 4);
  }
  return found_labels;
}

static Register *makeRegStruct(char *regString) {
  Register *reg = malloc(sizeof(Register));
  reg->is_64_bit = regString[0] == 'x';
  reg->register_number = strtol(++regString, NULL, 10);
  return reg;
}

static Shift *
makeShiftStruct(InstructionToken shiftType, ImmediateToken magnitude) {
  Shift *shift = malloc(sizeof(Shift));
  shift->type = shiftType.instruction;
  shift->amount = magnitude.value;
  return shift;
}

Parser_Tree *second_pass(ArrayList *list, Map *tree) {//why return pointer?
  Parser_Tree *returnTree = malloc(sizeof(Parser_Tree));
  for (int i = 0; i < list->size; i++) {
    ArrayList *line = get_ArrayList_element(list, i);
    Token first_token = get_ArrayList_element(line, 0);
    if (first_token->type == TOKEN_TYPE_LABEL) {//label
      break;

    } else if (first_token->type == TOKEN_TYPE_NOP) {//nop
      returnTree->type = Type_nop;

    } else if (first_token->type == TOKEN_TYPE_DOT_INT) {//.int
      returnTree->type = Type_b_bCond_dotInt;
      returnTree->b_bCond_dotInt.instruction = first_token->instructionToken.instruction;
      returnTree->b_bCond_dotInt.immediate = ((Token) get_ArrayList_element(
              line, 0))->immediateToken.value;
      return returnTree;

    } else if (first_token->type == TOKEN_TYPE_INSTRUCTION) {
      Token second_token = get_ArrayList_element(line, 1);
      if (second_token->type == TOKEN_TYPE_IMMEDIATE) {//b, b.cond
        returnTree->type = Type_b_bCond_dotInt;
        returnTree->b_bCond_dotInt.instruction = first_token->instructionToken.instruction;
        returnTree->b_bCond_dotInt.immediate = ((Token) get_ArrayList_element(
                line, 0))->immediateToken.value;
        return returnTree;

      } else if (second_token->type == TOKEN_TYPE_REGISTER) {
        if (line->size == 2) {//br
          returnTree->type = Type_br;
          returnTree->br.instruction = first_token->instructionToken.instruction;
          returnTree->br.Rn = *makeRegStruct(
                  second_token->registerToken.register_name);
        } else {
          Token third_token = get_ArrayList_element(line, 2);
          if (third_token->type ==
              TOKEN_TYPE_IMMEDIATE) {//cmp, cmn, neg, negs, movk, movn, movz, ldr
            returnTree->type = Type_cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM;
            returnTree->cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM.instruction = first_token->instructionToken.instruction;
            returnTree->cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM.R1 = *makeRegStruct(
                    ((Token) get_ArrayList_element(line,
                                                   0))->registerToken.register_name);
            if (line->size == 5) {
              returnTree->cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM.shift = makeShiftStruct(
                      ((Token) get_ArrayList_element(line,
                                                     3))->instructionToken,
                      ((Token) get_ArrayList_element(line, 4))->immediateToken);
            }
            return returnTree;

          } else if (third_token->type == TOKEN_TYPE_REGISTER) {
            if (line->size == 3 || line->size == 5 &&
                                   ((Token) get_ArrayList_element(line,
                                                                  3))->type ==
                                   TOKEN_TYPE_INSTRUCTION) {//cmp,cmn,neg, negs, tst, mov, mvn
              returnTree->type = Type_cmp_cmn_neg_negs_tst_mov_mvn_REG;
              returnTree->cmp_cmn_neg_negs_tst_mov_mvn_REG.instruction = first_token->instructionToken.instruction;
              returnTree->cmp_cmn_neg_negs_tst_mov_mvn_REG.R1 = *makeRegStruct(
                      second_token->registerToken.register_name);
              returnTree->cmp_cmn_neg_negs_tst_mov_mvn_REG.Rm = *makeRegStruct(
                      third_token->registerToken.register_name);
              if (line->size == 5) {
                returnTree->cmp_cmn_neg_negs_tst_mov_mvn_REG.shift = makeShiftStruct(
                        ((Token) get_ArrayList_element(line,
                                                       3))->instructionToken,
                        ((Token) get_ArrayList_element(line,
                                                       4))->immediateToken);
              }
              return returnTree;
            } else {
              Token fourth_token = get_ArrayList_element(line, 3);
              if ((line->size == 4 || line->size == 6 &&
                                      ((Token) get_ArrayList_element(line,
                                                                     4))->type ==
                                      TOKEN_TYPE_INSTRUCTION)) {//logic, mul, mneg
                returnTree->type = Type_logical_mul_mneg;
                returnTree->logical_mul_mneg.instruction = first_token->instructionToken.instruction;
                returnTree->logical_mul_mneg.Rd = *makeRegStruct(
                        second_token->registerToken.register_name);
                returnTree->logical_mul_mneg.Rn = *makeRegStruct(
                        third_token->registerToken.register_name);
                returnTree->logical_mul_mneg.Rm = *makeRegStruct(
                        fourth_token->registerToken.register_name);
                if (line->size == 6) {
                  returnTree->cmp_cmn_neg_negs_movk_movn_movz_ldrlit_IMM.shift = makeShiftStruct(
                          ((Token) get_ArrayList_element(line,
                                                         4))->instructionToken,
                          ((Token) get_ArrayList_element(line,
                                                         5))->immediateToken);
                }
                return returnTree;
              } else if (line->size == 5 &&
                         ((Token) get_ArrayList_element(line, 4))->type ==
                         TOKEN_TYPE_REGISTER) {//madd, msub
                returnTree->type = Type_madd_msub;
                returnTree->madd_msub.instruction = first_token->instructionToken.instruction;
                returnTree->madd_msub.Rd = *makeRegStruct(
                        second_token->registerToken.register_name);
                returnTree->madd_msub.Rn = *makeRegStruct(
                        third_token->registerToken.register_name);
                returnTree->madd_msub.Rm = *makeRegStruct(
                        fourth_token->registerToken.register_name);
                returnTree->madd_msub.Ra = *makeRegStruct(
                        ((Token) get_ArrayList_element(line,
                                                       4))->registerToken.register_name);
              } else {
                //error
              }
            }
          }
        }
      } else {
        //error
      }
    } else {
      //error
    }
  }
}