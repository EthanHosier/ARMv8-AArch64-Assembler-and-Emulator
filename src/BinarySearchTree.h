#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <stdint.h>

struct strbst; // forward definition
typedef struct strbst *BinarySearchTree; // pure binary tree of (sorted) strings
struct strbst {
    char *key; // the key: a string
    BinarySearchTree left, right; // the left and right subtrees
};

#endif