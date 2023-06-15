#include "parser.h"
#include "../../Map.h"
#include <stdlib.h>

Map *first_pass(ArrayList *list) {
  if (list == NULL || list->size == 0) return NULL;
  Map *found_labels = create_map();
  for (int i = 0; i < list->size; i++) {
    ArrayList *line = get_ArrayList_element(list, i);
    if (line == NULL || line->size == 0) return NULL;
    Token first_token = get_ArrayList_element(line, 0);
    if (first_token->type == TOKEN_TYPE_LABEL)
      put_map(found_labels, first_token->labelToken.label, i*4);
  }
  return found_labels;
}

Parser_Tree *second_pass(ArrayList *list, Map *tree) {

}