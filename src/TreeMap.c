#include "TreeMap.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ArrayList.h"

#include "global.h"

#define CHECK_CHILD_NODE(t)\
if((t) == NULL) {\
  (t) = new;\
  return;\
}\
current = (t)


static Binary_search_tree *create_node(const char *key, void *value) {
  Binary_search_tree *tree = malloc(sizeof(Binary_search_tree));
  if (tree == NULL) return NULL;
  tree->value = value;
  tree->key = key;
  tree->left = NULL;
  tree->right = NULL;
  return tree;
}

TreeMap *create_map(free_map_element free_element) {
  TreeMap *new = malloc(sizeof(TreeMap));
  if (new == NULL) return NULL;
  new->root = NULL;
  new->free_element = free_element;
  return new;
}

void put_map(TreeMap *map, const char *key, void *value) {
  assert(value != NULL);
  Binary_search_tree *new = create_node(key, value);
  if (new == NULL) {
    IRREPARABLE_MEMORY_ERROR;
  }
  if (map->root == NULL) {
    map->root = new;
    return;
  }
  Binary_search_tree *current = map->root;
  for (;;) {
    int compared = strcmp(key, current->key);
    if (compared == 0) {
      current->value = value;
      return;
    }
    if (compared < 0) {
      CHECK_CHILD_NODE(current->left);
    } else {
      CHECK_CHILD_NODE(current->right);
    }
  }
}

void *get_map(TreeMap *map, char *key) {
  Binary_search_tree *current = map->root;
  for (;;) {
    if (current == NULL) return NULL;
    int compared = strcmp(key, current->key);
    if (compared == 0) return current->value;
    if (compared < 0) current = current->left;
    else current = current->right;
  }
}

bool in_map(TreeMap *map, char *key) {
  return get_map(map, key) != NULL;
}

void free_map(void *input) {
  TreeMap *map = (TreeMap *) input;
  ArrayList *stack1 = create_ArrayList(NULL, NULL);
  if (stack1 == NULL) {
    IRREPARABLE_MEMORY_ERROR;
  }
  ArrayList *stack2 = create_ArrayList(NULL, NULL);
  if (stack2 == NULL) {
    free_ArrayList(stack1);
    IRREPARABLE_MEMORY_ERROR;
  }

  add_ArrayList_element(stack1, map->root);
  while (stack1->size != 0) {
    Binary_search_tree *node = remove_ArrayList_element(stack1);
    add_ArrayList_element(stack2, node);
    if (node->left != NULL) add_ArrayList_element(stack1, node->left);
    if (node->right != NULL) add_ArrayList_element(stack1, node->right);
  }
  while (stack2->size != 0) {
    Binary_search_tree *to_free = remove_ArrayList_element(stack2);
    if (map->free_element != NULL) (map->free_element)(to_free->value);
    free(to_free);
  }
  free_ArrayList(stack1);
  free_ArrayList(stack2);
  free(map);
}