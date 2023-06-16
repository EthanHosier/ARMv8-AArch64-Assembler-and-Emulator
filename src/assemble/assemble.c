#include "assemble.h"
#include <stdlib.h>
#include <printf.h>

int main(int argc, char **argv) {
    char str[] = "ldr x20, [x5, #8]";
    ArrayList *tokens = tokenize(str);
    print_ArrayList_elements(tokens);

  char **lines = NULL; //readLines(file_name);
  ArrayList *token_lines = create_ArrayList(NULL, &free_ArrayList);
  for (int i = 0; i < sizeof(lines); i++) // maybe works
    add_ArrayList_element(token_lines, tokenize(lines[i]));
  TreeMap *label_identifiers = first_pass(token_lines);
  ArrayList *trees = second_pass(token_lines, label_identifiers);
  free_ArrayList(token_lines);
  free_map(label_identifiers);
  // TODO: Make second_pass return an ArrayList
  // TODO: Use the parser trees to create machine code
  free_ArrayList(trees);
  return EXIT_SUCCESS;
}
