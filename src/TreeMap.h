#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct binary_search_tree Binary_search_tree;
struct binary_search_tree {
  void *key;
  void *value;
  Binary_search_tree *left;
  Binary_search_tree *right;
};
typedef void (*free_map_element)(void *);
typedef int (*compare_map_keys)(void *, void *);
typedef struct {
  Binary_search_tree *root;
  free_map_element free_values;
  free_map_element free_keys;
  compare_map_keys compare_keys;
} TreeMap;

extern TreeMap *create_map(free_map_element free_keys,
                           free_map_element free_values,
                           compare_map_keys);

extern void put_map(TreeMap *map, void *key, void *value);

extern void *get_map(TreeMap *map, void *key);

extern void free_map(void *map);

extern bool in_map(TreeMap *map, void *key);

extern int compare_strings_map(void *, void *);

extern int get_map_int(TreeMap *map, void *key);

extern void put_map_int(TreeMap *map, void *key, int value);

extern void put_map_int_key(TreeMap *map, int key, void *value);

extern void *get_map_int_key(TreeMap *map, int key);

#endif
