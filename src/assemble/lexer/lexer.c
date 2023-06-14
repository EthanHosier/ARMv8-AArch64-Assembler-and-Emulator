
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

static void initialiseInstructionsBST() {
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
        add_strbst(instructionsBST, instructions[i], -1);
    }
}

static void print_arrayList_element(void *element) {
    char strs[] = {"INSTRUCTION",
                   "IMMEDIATE",
                   "LABEL",
                   "DOT_INT",
                   "ADDRESS_CODE"};

    printf("%s", strs[((Token) element)->type]);
}


static Token string_to_token(char *str) {
    fprintf(stdout, "%s\n", str);

    Token t = NEW(struct Token);
    assert(t != NULL);

    if (strcmp(str, ".int") == 0) {
        t->type = TOKEN_TYPE_DOT_INT;
    }

        //check for instruciton token
    else if (in_strbst(instructionsBST, str)) {
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


        //check if is address code
        switch (check_if_address_code(tokenStrCopy)) {
            case ADDRESS_CODE_POSSIBILITIES_NOT: {
                Token t = string_to_token(tokenStrCopy);
                add_ArrayList_element(tokens, t);
                break;
            }


            case ADDRESS_CODE_POSSIBILITIES_ONE:{
                Token t = NEW(struct Token);
                assert(t != NULL);

                AddressCodeToken *act = NEW (AddressCodeToken);
                assert(act != NULL);

                Token tInside = string_to_token(strtok(tokenStrCopy," []!"));

                act-> t1 = tInside;

                t->type = TOKEN_ADDRESS_CODE;
                t->addressToken = *act;


                break;
            }

            case ADDRESS_CODE_POSSIBILITIES_TWO: {
                Token t = NEW (struct Token);
                assert(t != NULL);

                AddressCodeToken *act = NEW (AddressCodeToken);
                assert(act != NULL);

                Token t1 = string_to_token(strtok(tokenStrCopy," []!"));
                tokenStr = strtok(NULL, " ,");
                char *tokenStrCopy = strdup(tokenStr);
                Token t2 = string_to_token(strtok(tokenStrCopy," []!"));

                act->t1 = t1;
                act->pT2 = t2;


                t->type = TOKEN_ADDRESS_CODE;
                t->addressToken = *act;

            }
        }

        tokenStr = strtok(NULL, " ,");

    }

    return tokens;


}
