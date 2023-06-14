#include "parser.h"
#include <stdlib.h>

BinarySearchTree first_pass(ArrayList *list) {
  if (list == NULL || list->size == 0) return NULL;
  for (int i = 0; i < list->size; i++) {
    ArrayList *line = get_ArrayList_element(list, i);
    if (line == NULL || line->size == 0) return NULL;

  }
}

Parser_Tree *second_pass(ArrayList *list, BinarySearchTree tree) {

}