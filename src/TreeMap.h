#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct binary_search_tree Binary_search_tree;
struct binary_search_tree {
  const char *key;
  void *value;
  Binary_search_tree *left;
  Binary_search_tree *right;
};
typedef void (*free_map_element)(void *);
typedef struct {
  Binary_search_tree *root;
  free_map_element free_element;
} TreeMap;

extern TreeMap *create_map(free_map_element);

extern void put_map(TreeMap *map, const char *key, void *value);

extern void *get_map(TreeMap *map, char *key);

extern void free_map(void *map);

extern bool in_map(TreeMap *map, char *key);

#endif
