#include "assemble.h"
#include "../Map.h"
#include <stdlib.h>
#include <printf.h>

int main(int argc, char **argv) {
  Token label1 = malloc(sizeof(Token));
  LabelToken *specificLabel = malloc(sizeof(LabelToken));
  specificLabel->label = "foo";
  label1->type = TOKEN_TYPE_LABEL;
  label1->labelToken = *specificLabel;
  ArrayList *lines = create_ArrayList(NULL, NULL);
  ArrayList *firstLine = create_ArrayList(NULL, NULL);
  add_ArrayList_element(lines, firstLine);
  add_ArrayList_element(firstLine, label1);
  Map *labels = first_pass(lines);
  printf("hey");

  return EXIT_SUCCESS;
}
