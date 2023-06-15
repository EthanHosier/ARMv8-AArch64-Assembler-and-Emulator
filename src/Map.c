#include "Map.h"
#include <stdlib.h>
#include <string.h>

#define CHECK_CHILD_NODE(t)\
if((t) == NULL) {\
  (t) = new;\
  return true;\
}\
current = (t)


static Binary_search_tree *create_node(char *key, uint32_t value) {
  Binary_search_tree *tree = malloc(sizeof(Binary_search_tree));
  if (tree == NULL) return NULL;
  tree->value = value;
  tree->key = key;
  tree->left = NULL;
  tree->right = NULL;
  return tree;
}

Map *create_map() {
  Map *new = malloc(sizeof(Map));
  if (new == NULL) return NULL;
  new->root = NULL;
  return new;
}

bool put_map(Map *map, char *key, uint32_t value) {
  Binary_search_tree *new = create_node(key, value);
  if (new == NULL) return false;
  if (map->root == NULL) {
    map->root = new;
    return true;
  }
  Binary_search_tree *current = map->root;
  for (;;) {
    int compared = strcmp(key, current->key);
    if (compared == 0) {
      current->value = value;
      return true;
    }
    if (compared < 0) {
      CHECK_CHILD_NODE(current->left);
    } else {
      CHECK_CHILD_NODE(current->right);
    }
  }
}

bool in_map(Map *map, char *key) {
  Binary_search_tree *current = map->root;
  for (;;) {
    if (current == NULL) return false;
    int compared = strcmp(key, current->key);
    if (compared == 0) return true;
    if (compared < 0) current = current->left;
    else current = current->right;
  }
}

uint32_t getVal_map(Map *map, char *key) {
  Binary_search_tree *current = map->root;
  for (;;) {
    if (current == NULL) return false;
    int compared = strcmp(key, current->key);
    if (compared == 0) return current->value;
    if (compared < 0) current = current->left;
    else current = current->right;
  }
}

static void free_bst(Binary_search_tree *bst) {
  if (bst == NULL) return;
  free_bst(bst->left);
  free_bst(bst->right);
  free(bst);
}

void free_map(Map *map) {
  free_bst(map->root);
  free(map);
}