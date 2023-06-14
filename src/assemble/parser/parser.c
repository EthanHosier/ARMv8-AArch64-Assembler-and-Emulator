#include "parser.h"
#include <stdlib.h>

Parser_Tree *parse(ArrayList *list) {
  static uint32_t unknown_identifiers;
  if (get_ArrayList_element(list, 0) == NULL) return NULL;

}