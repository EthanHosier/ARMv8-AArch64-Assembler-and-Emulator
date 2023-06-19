
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include "lexer.h"
#include "../../TreeMap.h"

#define NEW(t) malloc(sizeof(t))

static TreeMap *instructionsBST = NULL;

static void free_token(void *input) {
  if (input == NULL) return;
  Token token = (Token) input;
  switch (token->type) {
    case TOKEN_TYPE_INSTRUCTION:
      free(token->instructionToken.instruction);
      break;
    case TOKEN_TYPE_REGISTER:
      free(token->registerToken.register_name);
      break;
    case TOKEN_TYPE_LABEL:
      free(token->labelToken.label);
      break;
    case TOKEN_ADDRESS_CODE:
      if (token->addressToken.t1->type == TOKEN_TYPE_REGISTER) {
        free(token->addressToken.t1->registerToken.register_name);
        free(token->addressToken.t1);
      }
      free_token(token->addressToken.pT2);
      break;
    default:
      break;
  }
  free(token);
}

static void initialiseInstructionsBST(void) {
  instructionsBST = create_map(NULL, free, compare_strings_map);
  if (instructionsBST == NULL) {
    fprintf(stderr, "Not enough memory to assemble!");
    exit(1);
  }
  char *instructions[] = {
      "add",
      "adds",
      "sub",
      "subs",
      "cmp",
      "cmn",
      "neg",
      "negs",
      "and",
      "ands",
      "bic",
      "bics",
      "eor",
      "orr",
      "eon",
      "orn",
      "tst",
      "movk",
      "movn",
      "movz",
      "mov",
      "mvn",
      "madd",
      "msub",
      "mul",
      "mneg",
      "b",
      "b.eq",
      "b.ne",
      "b.ge",
      "b.lt",
      "b.gt",
      "b.le",
      "b.al",
      "br",
      "ldr",
      "str",
      "nop",
      "lsl",
      "lsr",
      "asr",
      "ror"
  };
  for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
    put_map_int(instructionsBST, instructions[i], -1);
  }
}

static void print_Token(void *element) {
  Token t = (Token) element;

  TreeMap *map = create_map(free, NULL, compare_ints_map);
  put_map_int_key(map, TOKEN_TYPE_IMMEDIATE, "Immediate");
  put_map_int_key(map, TOKEN_TYPE_INSTRUCTION, "Instruction");
  put_map_int_key(map, TOKEN_TYPE_REGISTER, "Register");
  put_map_int_key(map, TOKEN_TYPE_LABEL, "Label");
  put_map_int_key(map, TOKEN_TYPE_DOT_INT, ".int");
  char *type = get_map_int_key(map, t->type);
  if (t->type == TOKEN_ADDRESS_CODE) {
    printf("Address@[%s: %s",
           (char *) get_map_int_key(map, t->addressToken.t1->type),
           t->addressToken.t1->registerToken.register_name);
    if (t->addressToken.pT2 != NULL) {
      char *pt2Type = get_map_int_key(map, t->addressToken.pT2->type);
      if (t->addressToken.pT2->type == TOKEN_TYPE_REGISTER) {
        printf(", %s: %s",
               pt2Type,
               t->addressToken.pT2->registerToken.register_name);
      } else if (t->addressToken.pT2->type == TOKEN_TYPE_IMMEDIATE) {
        printf(", %s: %d",
               pt2Type,
               t->addressToken.pT2->immediateToken.value);
      }
    }
    printf("]");
    if (t->addressToken.exclamation) printf("!");

  } else {
    printf("%s: ", type);
    if (t->type == TOKEN_TYPE_INSTRUCTION)
      printf("%s",
             t->instructionToken.instruction);
    else if (t->type == TOKEN_TYPE_REGISTER)
      printf("%s", t->registerToken.register_name);
    else if (t->type == TOKEN_TYPE_IMMEDIATE)
      printf("%"PRIu32, t->immediateToken.value);
    else if (t->type == TOKEN_TYPE_LABEL)
      printf("%s", t->labelToken.label);
  }
  free_map(map);
}

static uint32_t *is_convertible_to_int(const char *str) {
  char *endptr;
  uint32_t *num = malloc(sizeof(uint32_t));
  *num = strtol(str, &endptr, 10);
  if (str == endptr || *endptr != '\0') {
    free(num);
    return NULL;
  }
  return num;
}

static Token string_to_token(char *str) {
  //fprintf(stdout, "%s\n", str);

  Token t = NEW(struct Token);
  assert(t != NULL);
  uint32_t *num;
  if (strcmp(str, ".int") == 0) {
    t->type = TOKEN_TYPE_DOT_INT;
    free(str);
  }

    //check for instruction token
  else if (in_map(instructionsBST, str)) {
    t->instructionToken.instruction = str;
    t->type = TOKEN_TYPE_INSTRUCTION;
  }

    //check for '#' immediate token
  else if (str[0] == '#' || str[0] == '0') {
    char *numStr= (str[0] == '#') ? str+1 : str;
    if (strlen(numStr) > 1 && numStr[1] == 'x') {
      t->immediateToken.value = (uint32_t) strtol(numStr, NULL, 16);
    } else {
      t->immediateToken.value = (uint32_t) strtol(numStr, NULL, 10);
    }
    free(str);
    t->type = TOKEN_TYPE_IMMEDIATE;
  }

    //check for register token
  else if (strcmp("xzr", str) == 0 || strcmp("wzr", str) == 0
      || ((str[0] == 'w' || str[0] == 'x')
          && ((int) str[1] <= 57 && (int) str[1] >= 48))) {
    t->registerToken.register_name = str;
    t->type = TOKEN_TYPE_REGISTER;
  }

    // 'immediate' value of a .int directive
  else if ((num = is_convertible_to_int(str)) != NULL) {
    t->type = TOKEN_TYPE_IMMEDIATE;
    t->immediateToken.value = *num;
    free(num);
    free(str);
  }

    //treat as a label
  else {
    // TODO: if token is first token on the line- check to ensure there is a colon
    t->labelToken.label = str;
    t->type = TOKEN_TYPE_LABEL;
  }

  return t;

}

static AddressCodePossibilities
check_if_address_code(char *str) {
  if (str[0] != '[') {
    return ADDRESS_CODE_POSSIBILITIES_NOT;
  }

  int length = (int) strlen(str);

  if (str[length - 1] == ']') {
    return ADDRESS_CODE_POSSIBILITIES_ONE;
  }

  return ADDRESS_CODE_POSSIBILITIES_TWO;
}

static ArrayList *tokenize_line(char *line) {
  initialiseInstructionsBST();

  ArrayList *tokens = create_ArrayList(print_Token, free_token);
  char *tokenStr;

  tokenStr = strtok(line, " ");
  for(;;) {
    // Create a copy of the token
    char *tokenStrCopy = strdup(tokenStr);

    AddressCodePossibilities
        result = check_if_address_code(tokenStrCopy);
    Token t;

    if (result == ADDRESS_CODE_POSSIBILITIES_NOT) {
      t = string_to_token(tokenStrCopy);
    } else {
      t = NEW(struct Token);
      assert(t != NULL);
      char *new_value = strdup(tokenStrCopy + 1); //remove the "\["
      Token t1 = string_to_token(new_value);
      free(tokenStrCopy);
      if (result == ADDRESS_CODE_POSSIBILITIES_ONE) {
        t->addressToken.pT2 = NULL;
        new_value[strlen(new_value) - 1] = '\0';
      } else {
        //ADDRESS_CODE_POSSIBILITIES_TWO
        tokenStr = strtok(NULL, " ,]");
        tokenStrCopy = strdup(tokenStr);
        Token t2 = string_to_token(tokenStrCopy);

        t->addressToken.pT2 = t2;

      }
      t->addressToken.t1 = t1;
      t->addressToken.exclamation = false;
      t->type = TOKEN_ADDRESS_CODE;
    }
    add_ArrayList_element(tokens, t);
    tokenStr = strtok(NULL, " ,");
    if (tokenStr == NULL) break;
    if (strcmp(tokenStr, "!") == 0) {
      t->addressToken.exclamation = true;
      break;
    }
  }
  free_map(instructionsBST);
  return tokens;

}


ArrayList *tokenize(ArrayList *lines) {
  ArrayList *new = create_ArrayList(print_ArrayList_elements, free_ArrayList);
  for (int i = 0; i < lines->size; i++) {// maybe works
    void *element = get_ArrayList_element(lines, i);
    ArrayList *tokens = tokenize_line(element);
    add_ArrayList_element(new, tokens);
    print_ArrayList_elements(tokens);
  }
  return new;
}

