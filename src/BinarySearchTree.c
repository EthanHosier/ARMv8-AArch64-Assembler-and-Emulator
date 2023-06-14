#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "BinarySearchTree.h"

#define NEW(t) malloc(sizeof(struct t))


// BinarySearchTree t = makenode( key );
// Make a new bst node with <key>, duplicating the key.
// Abort if any memory allocation fails.
static BinarySearchTree makenode(char *key, uint32_t value) {
  BinarySearchTree t = NEW(strbst);
  assert(t != NULL);
  key = strdup(key); // key = strcpy( malloc(1+strlen(key), key )
  assert(key != NULL);
  t->key = key;
  t->left = NULL;
  t->right = NULL;
  t->value = value;
  return t;
}

// bool present = in_strbst( t, key );
// Return true iff <key> is in <t>. Else return false.
bool in_strbst(BinarySearchTree t, char *key) {
  while (t != NULL) {
    int cmp = strcmp(key, t->key);
    if (cmp == 0) return true;
    t = (cmp < 0) ? t->left : t->right;
  }
  return false;
}

// t = add_strbst( t, key );
// Add <key> to <t>, if it’s not already present.
// Abort if any memory allocation fails.
BinarySearchTree add_strbst(BinarySearchTree t, char *key, uint32_t value) {
  if (t == NULL) {
    return makenode(key, value);
  }
  int cmp = strcmp(key, t->key);
  if (cmp == 0) {
// already present
    return t;
  }
  if (cmp < 0) {
    t->left = add_strbst(t->left, key, value);
  } else {
    t->right = add_strbst(t->right, key, value);
  }
  return t;
}

// free_strbst( t );
// Free the given BinarySearchTree <t>.
void free_strbst(BinarySearchTree t) {
  if (t->left != NULL) free_strbst(t->left);
  if (t->right != NULL) free_strbst(t->right);
  free(t->key); // was strdup()ed, remember
  free(t);
}
