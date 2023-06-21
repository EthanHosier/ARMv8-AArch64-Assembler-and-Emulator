#include "TreeMap.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ArrayList.h"

#include "global.h"

#define CHECK_CHILD_NODE(t)\
if((t) == NULL) {\
  (t) = new;\
  executing = false;\
}\
current = (t)


static Binary_search_tree *create_node(void *key, void *value) {
  Binary_search_tree *tree = malloc(sizeof(Binary_search_tree));
  if (tree == NULL) return NULL;
  tree->value = value;
  tree->key = key;
  tree->left = NULL;
  tree->right = NULL;
  return tree;
}

TreeMap *create_map(free_map_element free_keys,
                    free_map_element free_values,
                    compare_map_keys compare_keys) {
  TreeMap *new = malloc(sizeof(TreeMap));
  if (new == NULL) return NULL;
  new->root = NULL;
  new->free_values = free_values;
  new->free_keys = free_keys;
  new->compare_keys = compare_keys;
  return new;
}

static Binary_search_tree *avl_rebalance(Binary_search_tree *);

void put_map(TreeMap *map, void *key, void *value) {
  assert(value != NULL);
  Binary_search_tree *new = create_node(key, value);
  if (new == NULL) {
    IRREPARABLE_MEMORY_ERROR;
  }
  bool executing = true;
  if (map->root == NULL) {
    map->root = new;
    executing = false;
  }
  Binary_search_tree *current = map->root;
  while (executing) {
    int compared = (map->compare_keys)(key, current->key);
    if (compared == 0) {
      current->value = value;
      executing = false;
    } else if (compared < 0) {
      CHECK_CHILD_NODE(current->left);
    } else {
      CHECK_CHILD_NODE(current->right);
    }
  }
  map->root = avl_rebalance(map->root);
}

void put_map_int(TreeMap *map, void *key, int value) {
  int *pointer = malloc(sizeof(int));
  *pointer = value;
  put_map(map, key, pointer);
}

void *get_map(TreeMap *map, void *key) {
  Binary_search_tree *current = map->root;
  for (;;) {
    if (current == NULL) return NULL;
    int compared = (map->compare_keys)(key, current->key);
    if (compared == 0) return current->value;
    if (compared < 0) current = current->left;
    else current = current->right;
  }
}

int get_map_int(TreeMap *map, void *key) {
  void *result = get_map(map, key);
  assert(result != NULL);
  return *(int *) result;
}

bool in_map(TreeMap *map, void *key) {
  return get_map(map, key) != NULL;
}

void free_map(void *input) {
  if (input == NULL) return;
  TreeMap *map = (TreeMap *) input;
  if (map->root == NULL) {
    free(map);
    return;
  }
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
    if (map->free_values != NULL) (map->free_values)(to_free->value);
    if (map->free_keys != NULL) (map->free_keys)(to_free->key);
    free(to_free);
  }
  free_ArrayList(stack1);
  free_ArrayList(stack2);
  free(map);
}

int compare_strings_map(void *key1, void *key2) {
  return strcmp(key1, key2);
}

void put_map_int_key(TreeMap *map, int key, void *value) {
  int *p = malloc(sizeof(int));
  *p = key;
  put_map(map, p, value);
}

void *get_map_int_key(TreeMap *map, int key) {
  return get_map(map, &key);
}

int compare_ints_map(void *input1, void *input2) {
  int *int1 = (int *) input1;
  int *int2 = (int *) input2;
  if (*int1 < *int2) return -1;
  if (*int1 > *int2) return 1;
  return 0;
}

static int get_height(Binary_search_tree *node) {
  if (node == NULL)
    return 0;
  return node->height;
}

static int max(int a, int b) {
  return (a > b) ? a : b;
}

static void update_height(Binary_search_tree *node) {
  if (node == NULL) return;
  node->height = 1 + max(get_height(node->left), get_height(node->right));
}

static Binary_search_tree *rotate_left(Binary_search_tree *node) {
  Binary_search_tree *new_root = node->right;
  node->right = new_root->left;
  new_root->left = node;
  update_height(node);
  update_height(new_root);
  return new_root;
}

static Binary_search_tree *rotate_right(Binary_search_tree *node) {
  Binary_search_tree *new_root = node->left;
  node->left = new_root->right;
  new_root->right = node;
  update_height(node);
  update_height(new_root);
  return new_root;
}

static Binary_search_tree *avl_rebalance(Binary_search_tree *node) {
  if (node == NULL) return NULL;
  node->left = avl_rebalance(node->left);
  node->right = avl_rebalance(node->right);
  update_height(node);
  int balance_factor = get_height(node->left) - get_height(node->right);
  // Left-Left case
  if (balance_factor > 1
      && get_height(node->left->left) >= get_height(node->left->right))
    return rotate_right(node);

  // Right-Right case
  if (balance_factor < -1
      && get_height(node->right->right) >= get_height(node->right->left))
    return rotate_left(node);

  // Left-Right case
  if (balance_factor > 1
      && get_height(node->left->left) < get_height(node->left->right)) {
    node->left = rotate_left(node->left);
    return rotate_right(node);
  }

  // Right-Left case
  if (balance_factor < -1
      && get_height(node->right->right) < get_height(node->right->left)) {
    node->right = rotate_right(node->right);
    return rotate_left(node);
  }

  return node; // No rebalancing needed
}
