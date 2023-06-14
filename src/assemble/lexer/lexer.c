#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "lexer.h"
#include "../../ArrayList.h"

#include "../../BinarySearchTree.h"

static BinarySearchTree instructionsBST;

static void ()

static void print_arrayList_element(void *element){
    fprintf(stdout, "willy");


}


static Token *checkIfInstruction(char *str){
    int RANGE = INT_MAX;



}


static Token *string_to_token(char *str){
    int RANGE = INT_MAX;


    fprintf(stdout, "%s\n",str);





    return NULL;
}


ArrayList *tokenize(char *line)
{
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
