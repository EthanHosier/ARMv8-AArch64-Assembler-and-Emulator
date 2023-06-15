#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct binary_search_tree Binary_search_tree;
struct binary_search_tree {
    char *key;
    uint32_t value;
    Binary_search_tree *left;
    Binary_search_tree *right;
};
typedef struct {
    Binary_search_tree *root;
} Map;

extern Map *create_map();

extern bool put_map(Map *map, char *key, uint32_t value);

extern bool in_map(Map *map, char *key);

uint32_t getVal_map(Map *map, char *key);

extern void free_map(Map *map);

#endif
