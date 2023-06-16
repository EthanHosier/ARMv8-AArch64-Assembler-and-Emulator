#ifndef TOKEN_H
#define TOKEN_H


#include <stdint.h>
#include "../../ArrayList.h"

typedef enum {
  TOKEN_TYPE_INSTRUCTION,
  TOKEN_TYPE_IMMEDIATE,
  TOKEN_TYPE_REGISTER,
  TOKEN_TYPE_LABEL,
  TOKEN_TYPE_DOT_INT,
  TOKEN_ADDRESS_CODE,
} TokenType;

typedef enum {
  ADDRESS_CODE_POSSIBILITIES_NOT,
  ADDRESS_CODE_POSSIBILITIES_ONE,
  ADDRESS_CODE_POSSIBILITIES_TWO
} AddressCodePossibilities;

typedef struct {
  char *instruction;
} InstructionToken;


typedef struct {
  char *register_name;
} RegisterToken;


typedef struct {
  uint32_t value;
} ImmediateToken;

typedef struct {
  char *label;
} LabelToken;

struct Token;
typedef struct Token *Token;

typedef struct {
  Token t1;
  Token pT2;
} AddressCodeToken;


struct Token {
  TokenType type;
  union {
    InstructionToken instructionToken;
    RegisterToken registerToken;
    ImmediateToken immediateToken;
    LabelToken labelToken;
    AddressCodeToken addressToken;
  };
};


extern ArrayList *tokenize(char *line);

#endif
