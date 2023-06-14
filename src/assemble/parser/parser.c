#include "parser.h"
#include <stdlib.h>

BinarySearchTree first_pass(ArrayList *list) {
  if (list == NULL || list->size == 0) return NULL;
  BinarySearchTree found_labels = make_empty_strbst();
  for (int i = 0; i < list->size; i++) {
    ArrayList *line = get_ArrayList_element(list, i);
    if (line == NULL || line->size == 0) return NULL;
    Token *first_token = get_ArrayList_element(line, 0);
    if (first_token->type == TOKEN_TYPE_LABEL)
      add_strbst(found_labels, first_token->labelToken.label, i);
  }
  return found_labels;
}

Parser_Tree *second_pass(ArrayList *list, BinarySearchTree tree) {

}