#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>


typedef struct {
  char *instruction;
} InstructionToken;

typedef struct {
  char *register_name;
} RegisterToken;

typedef struct {
  uint32_t value;
} ConstantToken;


typedef enum {
  TOKEN_TYPE_INSTRUCTION,
  TOKEN_TYPE_IMMEDIATE,
  TOKEN_TYPE_REGISTER

} TokenType;

typedef struct {
  TokenType type;
  union {
    InstructionToken instructionToken;
    RegisterToken registerToken;
    ConstantToken constantToken;
  };
} Token;

extern Token *tokenize(char *line);

#endif
