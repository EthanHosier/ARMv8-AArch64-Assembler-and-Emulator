#ifndef TOKEN_H
#define TOKEN_H


#include <stdint.h>
#include "../../ArrayList.h"


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


typedef enum {
    TOKEN_TYPE_INSTRUCTION,
    TOKEN_TYPE_IMMEDIATE,
    TOKEN_TYPE_REGISTER,
    TOKEN_TYPE_LABEL,
    TOKEN_TYPE_DOT_INT
} TokenType;


typedef struct {
    TokenType type;
    union {
        InstructionToken instructionToken;
        RegisterToken registerToken;
        ImmediateToken immediateToken;
        LabelToken labelToken;
    };
} Token;


extern ArrayList *tokenize(char *line);

#endif
