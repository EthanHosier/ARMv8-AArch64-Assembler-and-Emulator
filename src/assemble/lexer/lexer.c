
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "lexer.h"
#include "../../Map.h"

#define NEW(t) malloc(sizeof(t))

static Map *instructionsBST = NULL;

static void initialiseInstructionsBST() {
  instructionsBST = create_map();
  if (instructionsBST == NULL) {
    fprintf(stderr, "Not enough memory to assemble!");
    exit(1);
  }
  const char *instructions[] = {
          "adds",
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
          "b",
          "br",
          "b.cond",
          "ldr",
          "str"
  };

  for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
    if (!put_map(instructionsBST, instructions[i], -1)) {
      fprintf(stderr, "Not enough memory to assemble!");
      exit(1);
    }
  }
}

static void print_arrayList_element(void *element) {
  char **strs = {"INSTRUCTION",
                 "IMMEDIATE",
                 "Register",
                 "LABEL",
                 "DOT_INT",
                 "ADDRESS_CODE"};
  int num = ((Token) element)->type;
  char *type = strs[num];
  printf("%s", type);
}


static Token string_to_token(char *str) {
  fprintf(stdout, "%s\n", str);

  Token t = NEW(struct Token);
  assert(t != NULL);

  if (strcmp(str, ".int") == 0) {
    t->type = TOKEN_TYPE_DOT_INT;
  }

    //check for instruciton token
  else if (in_map(instructionsBST, str)) {
    InstructionToken *instructionToken = NEW(InstructionToken);
    assert(instructionToken != NULL);

    instructionToken->instruction = str;
    t->instructionToken = *instructionToken;
    t->type = TOKEN_TYPE_INSTRUCTION;
  }

    //check for immediate token
  else if (str[0] == '#') {
    ImmediateToken *immediateToken = NEW (ImmediateToken);
    assert(immediateToken != NULL);

    immediateToken->value = (uint32_t) atoi(str);
    t->immediateToken = *immediateToken;
    t->type = TOKEN_TYPE_IMMEDIATE;
  }

    //check for register token
    else if ((str[0] == 'w' || str[0] == 'x') &&  ((int)str[1] <= 57 && (int)str[1] >= 48)){
        RegisterToken *registerToken = NEW (RegisterToken);
        assert(registerToken != NULL);

        registerToken->register_name = str;
        t->registerToken = *registerToken;
        t-> type = TOKEN_TYPE_REGISTER;
    }

    //treat as a label
  else {
    LabelToken *labelToken = NEW(LabelToken);
    assert(labelToken != NULL);

    labelToken->label = str;
    t->labelToken = *labelToken;
    t->type = TOKEN_TYPE_LABEL;
  }


  return t;

}

AddressCodePossibilities check_if_address_code(char *str) {
  if (str[0] != '[') {
    return ADDRESS_CODE_POSSIBILITIES_NOT;
  }

  int length = strlen(str);

  if (str[length - 1] == ']' || str[length - 1] == '!') {
    return ADDRESS_CODE_POSSIBILITIES_ONE;
  }

  return ADDRESS_CODE_POSSIBILITIES_TWO;
}

ArrayList *tokenize(char *line) {
  initialiseInstructionsBST();

  ArrayList *tokens = create_ArrayList(print_arrayList_element, &free);
  char *tokenStr;

  tokenStr = strtok(line, " ");
  while (tokenStr != NULL) {
    // Create a copy of the token
    char *tokenStrCopy = strdup(tokenStr);

    AddressCodePossibilities result = check_if_address_code(tokenStrCopy);
    Token t;

      if(result == ADDRESS_CODE_POSSIBILITIES_NOT){
          t = string_to_token(tokenStrCopy);
    } else {
          t = NEW(struct Token);
          assert(t != NULL);

          AddressCodeToken *act = NEW (AddressCodeToken);
          assert(act != NULL);
          Token t1 = string_to_token(strtok(tokenStrCopy, " []!"));

          if (result == ADDRESS_CODE_POSSIBILITIES_ONE) {
              act->t1 = t1;

              t->type = TOKEN_ADDRESS_CODE;
              t->addressToken = *act;

          } else{
              //ADDRESS_CODE_POSSIBILITIES_TWO

              tokenStr = strtok(NULL, " ,");
              tokenStrCopy = strdup(tokenStr);
              Token t2 = string_to_token(strtok(tokenStrCopy, " []!"));

              act->t1 = t1;
              act->pT2 = t2;


              t->type = TOKEN_ADDRESS_CODE;
              t->addressToken = *act;
          }

      }

      add_ArrayList_element(tokens, t);
      tokenStr = strtok(NULL, " ,");
  }
  free_map(instructionsBST);
  return tokens;


}
