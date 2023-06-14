#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "lexer.h"
#include "../../ArrayList.h"

#include "../../BinarySearchTree.h"

#define NEW(t) malloc(sizeof(t))

static BinarySearchTree instructionsBST;

static void initialiseInstructionsBST(){
    const char* instructions[] = {
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

    for (int i = 0; i < sizeof(instructions) / sizeof (instructions[0]); i++) {
        add_strbst(instructionsBST, instructions[i]);
    }
}

static void print_arrayList_element(void *element){
    fprintf(stdout, "willy");


}


static Token *string_to_token(char *str){
    fprintf(stdout, "%s\n",str);

    if (in_strbst(instructionsBST,str)){
        Token *t = NEW(Token);
        assert(t != NULL);

        InstructionToken *instructionToken = NEW(Token);
        assert(instructionToken != NULL);

        instructionToken->instruction = str;
        t->instructionToken = *instructionToken;
        t->type = TOKEN_TYPE_INSTRUCTION;
        return t;
    }



    return NULL;
}


ArrayList *tokenize(char *line)
{
    initialiseInstructionsBST();

    ArrayList *tokens = create_ArrayList(print_arrayList_element, &free);
    char *tokenStr;


    tokenStr = strtok(line, " ");
    while (tokenStr != NULL) {
        // Create a copy of the token and add it to the tokens list
        char *tokenStrCopy = strdup(tokenStr);
        Token *t = string_to_token(tokenStrCopy);
        add_ArrayList_element(tokens, t);


        tokenStr = strtok(NULL, " ");
    }


    return tokens;


}
