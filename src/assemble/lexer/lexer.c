
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "lexer.h"
#include "../../TreeMap.h"

#define NEW(t) malloc(sizeof(t))

static TreeMap *instructionsBST = NULL;

static void free_token(void *input) {
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
      "adds",
      "add",
      "sub",
      "cmp",
      "cmn",
      "neg",
      "and",
      "bic",
      "eor",
      "eon",
      "orr",
      "orn",
      "tst",
      "mvn",
      "mov",
      "movn",
      "movk",
      "movz",
      "madd",
      "msub",
      "mul",
      "mneg",
      "b.eq",
      "b.ne",
      "b.ge",
      "b.lt",
      "b.gt",
      "b.le",
      "b.al",
      "b",
      "br",
      "ldr",
      "str"
  };
  for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
    put_map_int(instructionsBST, instructions[i], -1);
  }
}

static void print_Token(void *element) {
  char **strs = malloc(6 * sizeof(char *));
  strs[0] = "INSTRUCTION";
  strs[1] = "IMMEDIATE";
  strs[2] = "REGISTER";
  strs[3] = "LABEL";
  strs[4] = "DOT_INT";
  strs[5] = "ADDRESS_CODE";

  Token t = (Token) element;
  int num = t->type;
  char *type = strs[num];

  if (strcmp(type, "ADDRESS_CODE") == 0) {
    printf("ADDRESS_CODE@[%s: %s",
           strs[(t->addressToken.t1->type)],
           t->addressToken.t1->registerToken.register_name);
    if (t->addressToken.pT2 != NULL) {
      char *pt2Type = strs[t->addressToken.pT2->type];
      if (strcmp(pt2Type, "REGISTER") == 0) {
        printf(", %s: %s",
               strs[(t->addressToken.pT2->type)],
               t->addressToken.pT2->registerToken.register_name);
      } else if (strcmp(pt2Type, "IMMEDIATE") == 0) {
        printf(", %s: %d",
               strs[(t->addressToken.pT2->type)],
               t->addressToken.pT2->immediateToken.value);
      }
    }
    printf("]");
    if (t->addressToken.exclamation) printf("!");

  } else {
    printf("%s", type);
  }

  free(strs);
}


static Token string_to_token(char *str) {
  fprintf(stdout, "%s\n", str);

  Token t = NEW(struct Token);
  assert(t != NULL);

  if (strcmp(str, ".int") == 0) {
    t->type = TOKEN_TYPE_DOT_INT;
  }

    //check for instruction token
  else if (in_map(instructionsBST, str)) {
    t->instructionToken.instruction = str;
    t->type = TOKEN_TYPE_INSTRUCTION;
  }

    //check for '#' immediate token
  else if (str[0] == '#' || str[0] == '0') {
    if (str[0] == '#') {
      str++; //get rid of '#'
      t->immediateToken.value = (uint32_t) atoi(str);
    } else {
      //str[0] == '0'
      t->immediateToken.value = (uint32_t) strtol(str, NULL, 16);
    }
    t->type = TOKEN_TYPE_IMMEDIATE;
  }


    //check for register token
  else if (strcmp("xzr", str) == 0 || strcmp("wzr", str) == 0
      || ((str[0] == 'w' || str[0] == 'x')
          && ((int) str[1] <= 57 && (int) str[1] >= 48))) {
    t->registerToken.register_name = str;
    t->type = TOKEN_TYPE_REGISTER;
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
check_if_address_code(char *str, bool *exclamation) {
  if (str[0] != '[') {
    return ADDRESS_CODE_POSSIBILITIES_NOT;
  }

  int length = (int) strlen(str);

  if (str[length - 1] == ']') {
    return ADDRESS_CODE_POSSIBILITIES_ONE;
  }

  if (str[length - 1] == '!') {
    *exclamation = 1;
    return ADDRESS_CODE_POSSIBILITIES_ONE;
  }

  return ADDRESS_CODE_POSSIBILITIES_TWO;
}

ArrayList *tokenize(char *line) {
  initialiseInstructionsBST();

  ArrayList *tokens = create_ArrayList(print_Token, free_token);
  char *tokenStr;

  tokenStr = strtok(line, " ");
  while (tokenStr != NULL) {
    // Create a copy of the token
    char *tokenStrCopy = strdup(tokenStr);

    bool exclamation = 0;
    AddressCodePossibilities
        result = check_if_address_code(tokenStrCopy, &exclamation);
    Token t;

    if (result == ADDRESS_CODE_POSSIBILITIES_NOT) {
      t = string_to_token(tokenStrCopy);
    } else {
      t = NEW(struct Token);
      assert(t != NULL);
      t->addressToken.exclamation = exclamation;
      tokenStrCopy++; //remove the "\["
      Token t1 = string_to_token(tokenStrCopy);
      if (result == ADDRESS_CODE_POSSIBILITIES_ONE) {
        int len = (int) strlen(tokenStrCopy);
        if (exclamation) {
          tokenStrCopy[len - 2] = '\0';
        } else {
          tokenStrCopy[len - 1] = '\0';
        }

      } else {
        //ADDRESS_CODE_POSSIBILITIES_TWO
        tokenStr = strtok(NULL, " ,]!");
        tokenStrCopy = strdup(tokenStr);
        Token t2 = string_to_token(strtok(tokenStrCopy, " ,]!"));

        t->addressToken.pT2 = t2;

      }
      t->addressToken.t1 = t1;
      t->type = TOKEN_ADDRESS_CODE;
    }
    add_ArrayList_element(tokens, t);
    tokenStr = strtok(NULL, " ,");
  }
  free_map(instructionsBST);
  return tokens;

}


