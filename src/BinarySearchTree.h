#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <stdint.h>

struct strbst; // forward definition
typedef struct strbst *strbst; // pure binary tree of (sorted) strings
struct strbst {
    char *key; // the key: a string
    strbst left, right; // the left and right subtrees
};

#endif