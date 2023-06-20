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
  ParserTree *tree = (ParserTree *) input;
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

static shift_type discriminatorShift(char *shift) {
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

static void
discriminator3args(ParserTree *tree, char *instr, bool isImmediate) {
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  if (isImmediate) {
    put_map_int(map, "add", Type_add_imm);
    put_map_int(map, "sub", Type_sub_imm);
    put_map_int(map, "adds", Type_adds_imm);
    put_map_int(map, "subs", Type_subs_imm);
  } else {
    put_map_int(map, "add", Type_add_reg);
    put_map_int(map, "sub", Type_sub_reg);
    put_map_int(map, "adds", Type_adds_reg);
    put_map_int(map, "subs", Type_subs_reg);
  }
  tree->type = get_map_int(map, instr);
  free_map(map);
}

static void
discriminator2args(ParserTree *tree, char *instr, bool isImmediate) {
  TreeMap *map = create_map(NULL, free, compare_strings_map);
  if (isImmediate) {
    put_map_int(map, "cmp", Type_cmp_imm);
    put_map_int(map, "cmn", Type_cmn_imm);
    put_map_int(map, "neg", Type_neg_imm);
    put_map_int(map, "negs", Type_negs_imm);
  } else {
    put_map_int(map, "cmp", Type_cmp_reg);
    put_map_int(map, "cmn", Type_cmn_reg);
    put_map_int(map, "neg", Type_neg_reg);
    put_map_int(map, "negs", Type_negs_reg);
  }
  tree->type = get_map_int(map, instr);
  free_map(map);
}

static void discriminatorLdrStr(ParserTree *tree,
                                char *instr,
                                Token third_token,
                                Token fourth_token) {
  if (fourth_token != NULL) {
    tree->type = (strcmp(instr, "ldr") == 0) ? Type_ldr_post : Type_str_post;
  } else if (third_token->type == TOKEN_TYPE_IMMEDIATE) {
    tree->type = Type_ldr_literal;
  } else if (third_token->addressToken.exclamation) {
    tree->type = (strcmp(instr, "ldr") == 0) ? Type_ldr_pre : Type_str_pre;
  } else if (third_token->addressToken.pT2 != NULL
      && third_token->addressToken.pT2->type == TOKEN_TYPE_REGISTER) {
    tree->type = (strcmp(instr, "ldr") == 0) ? Type_ldr_reg : Type_str_reg;
  } else {
    tree->type =
        (strcmp(instr, "ldr") == 0) ? Type_ldr_unsigned : Type_str_unsigned;
  }
}
static void bigBoyDiscriminator(ParserTree *tree,
                                Token first_token,
                                Token third_token,
                                Token fourth_token) {
  char *instr = first_token->instructionToken.instruction;
  if (strcmp(instr, "add") == 0 || strcmp(instr, "adds") == 0
      || strcmp(instr, "sub") == 0 || strcmp(instr, "subs") == 0) {
    discriminator3args(tree, instr, fourth_token->type == TOKEN_TYPE_IMMEDIATE);
  } else if ((strcmp(instr, "cmp") == 0 || strcmp(instr, "cmn") == 0
      || strcmp(instr, "neg") == 0 || strcmp(instr, "negs") == 0)) {
    discriminator2args(tree, instr, third_token->type == TOKEN_TYPE_IMMEDIATE);
  } else if (strcmp(instr, "ldr") == 0 || strcmp(instr, "str") == 0) {
    discriminatorLdrStr(tree, instr, third_token, fourth_token);
  } else {
    TreeMap *map = create_map(NULL, free, compare_strings_map);
    put_map_int(map, "and", Type_and);
    put_map_int(map, "ands", Type_ands);
    put_map_int(map, "bic", Type_bic);
    put_map_int(map, "bics", Type_bics);
    put_map_int(map, "eor", Type_eor);
    put_map_int(map, "orr", Type_orr);
    put_map_int(map, "eon", Type_eon);
    put_map_int(map, "orn", Type_orn);
    put_map_int(map, "tst", Type_tst);
    put_map_int(map, "movk", Type_movk);
    put_map_int(map, "movn", Type_movn);
    put_map_int(map, "movz", Type_movz);
    put_map_int(map, "mov", Type_mov);
    put_map_int(map, "mvn", Type_mvn);
    put_map_int(map, "madd", Type_madd);
    put_map_int(map, "msub", Type_msub);
    put_map_int(map, "mul", Type_mul);
    put_map_int(map, "mneg", Type_mneg);
    put_map_int(map, "b", Type_b);
    put_map_int(map, "b.eq", Type_beq);
    put_map_int(map, "b.ne", Type_bne);
    put_map_int(map, "b.ge", Type_bge);
    put_map_int(map, "b.lt", Type_blt);
    put_map_int(map, "b.gt", Type_bgt);
    put_map_int(map, "b.le", Type_ble);
    put_map_int(map, "b.al", Type_bal);
    put_map_int(map, "br", Type_br);
    tree->type = get_map_int(map, instr);
    free_map(map);
  }
}


static Register *makeRegStruct(char *regString) {
  Register *reg = malloc(sizeof(Register));
  if (reg == NULL) {
    IRREPARABLE_MEMORY_ERROR;
  }
  reg->is_64_bit = regString[0] == 'x';
  reg->register_number =
      (strcmp(regString, "xzr") == 0 || strcmp(regString, "wzr") == 0) ? 31
                                                                       : strtol(
          ++regString,
          NULL,
          10);
  return reg;
}

static Shift *
makeShiftStruct(InstructionToken shiftType, ImmediateToken magnitude) {
  Shift *shift = malloc(sizeof(Shift));
  if (shift == NULL) {
    IRREPARABLE_MEMORY_ERROR;
  }
  shift->type = discriminatorShift(shiftType.instruction);
  shift->amount = magnitude.value;
  return shift;
}

static ParserTree *make_parser_tree(void) {
  ParserTree *returnTree = malloc(sizeof(ParserTree));
  assert(returnTree != NULL);
  returnTree->R1 = NULL;
  returnTree->R2 = NULL;
  returnTree->R3 = NULL;
  returnTree->R4 = NULL;
  returnTree->shift = NULL;
  returnTree->imm = NULL;
  return returnTree;
}

static void replaceLabel(Token currTok, TreeMap *tree) {
  char *old_label_string = strdup(currTok->labelToken.label);
  free(currTok->labelToken.label);
  currTok->type = TOKEN_TYPE_IMMEDIATE;
  currTok->immediateToken.value = get_map_int(tree, old_label_string);
  free(old_label_string);
}

ArrayList *second_pass(ArrayList *file, TreeMap *tree) {//why return pointer?
  ArrayList *returnArray = create_ArrayList(NULL, free_parser_tree);
  for (int i = 0; i < file->size; i++) {

    ArrayList *line = get_ArrayList_element(file, i);
    Token first_token = get_ArrayList_element(line, 0);
    Token second_token = get_ArrayList_element(line, 1);
    Token third_token = get_ArrayList_element(line, 2);
    Token fourth_token = get_ArrayList_element(line, 3);

    //check for label def (do nothing)
    if (first_token->type == TOKEN_TYPE_LABEL)
      continue;

    //create return tree
    ParserTree *returnTree = make_parser_tree();

    //check for label reference in line (replace with memory location of label def)
    if (second_token != NULL
        && second_token->type == TOKEN_TYPE_LABEL)
      replaceLabel(second_token, tree);
    if (third_token != NULL
        && third_token->type == TOKEN_TYPE_LABEL)
      replaceLabel(third_token, tree);

    //deal with .int
    if (first_token->type == TOKEN_TYPE_DOT_INT) {
      returnTree->type = Type_dot_int;
      returnTree->imm = make_new_int(second_token->immediateToken.value);
      add_ArrayList_element(returnArray, returnTree);
      continue;
    }

    //handle incorrect assembly code syntax
    if (first_token->type != TOKEN_TYPE_INSTRUCTION) {
      perror("Incorrect syntax");
      exit(EXIT_FAILURE);
    }

    //check for nop
    if (strcmp(first_token->instructionToken.instruction, "nop") == 0) {
      returnTree->type = Type_nop;
      add_ArrayList_element(returnArray, returnTree);
      continue;
    }

    //allocate correct instruction type
    bigBoyDiscriminator(returnTree, first_token, third_token, fourth_token);

    //assigns to all in tree where they exist
    int n = 1;
    Token tempToken = get_ArrayList_element(line, n);
    if (tempToken->type == TOKEN_TYPE_REGISTER) {
      returnTree->R1 = makeRegStruct(tempToken->registerToken.register_name);
      tempToken = get_ArrayList_element(line, ++n);
      if (tempToken != NULL && tempToken->type == TOKEN_TYPE_REGISTER) {
        returnTree->R2 = makeRegStruct(tempToken->registerToken.register_name);
        tempToken = get_ArrayList_element(line, ++n);
        if (tempToken != NULL && tempToken->type == TOKEN_TYPE_REGISTER) {
          returnTree->R3 =
              makeRegStruct(tempToken->registerToken.register_name);
          tempToken = get_ArrayList_element(line, ++n);
          if (tempToken != NULL && tempToken->type == TOKEN_TYPE_REGISTER) {
            returnTree->R4 =
                makeRegStruct(tempToken->registerToken.register_name);
            tempToken = get_ArrayList_element(line, ++n);
          }
        }
      }
    }
    if (tempToken != NULL && tempToken->type == TOKEN_ADDRESS_CODE) {
      returnTree->R2 =
          makeRegStruct(tempToken->addressToken.t1->registerToken.register_name);
      if (tempToken->addressToken.pT2 == NULL) {
        returnTree->imm = make_new_int(0);
      } else if (tempToken->addressToken.pT2->type == TOKEN_TYPE_IMMEDIATE) {
        returnTree->imm =
            make_new_int(tempToken->addressToken.pT2->immediateToken.value);
      } else {
        returnTree->R3 =
            makeRegStruct(tempToken->addressToken.pT2->registerToken.register_name);
      }
      tempToken = get_ArrayList_element(line, ++n);
    }
    if (tempToken != NULL && tempToken->type == TOKEN_TYPE_IMMEDIATE) {
      if (returnTree->imm != NULL) free(returnTree->imm);
      returnTree->imm = make_new_int(tempToken->immediateToken.value);
      tempToken = get_ArrayList_element(line, ++n);
    }
    if (tempToken != NULL && tempToken->type == TOKEN_TYPE_INSTRUCTION) {
      InstructionToken shiftTypeToken = tempToken->instructionToken;
      tempToken = get_ArrayList_element(line, ++n);
      ImmediateToken shiftMagToken = tempToken->immediateToken;
      returnTree->shift = makeShiftStruct(shiftTypeToken, shiftMagToken);
    }
    add_ArrayList_element(returnArray, returnTree);
  }
  return returnArray;
}