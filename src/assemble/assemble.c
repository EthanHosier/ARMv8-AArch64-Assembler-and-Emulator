#include "assemble.h"
#include <stdlib.h>
#include <printf.h>

int main(int argc, char **argv) {
  char line[] = "add x0, x0, x0";
  ArrayList *tokens = tokenize(line);
  print_ArrayList_elements(tokens);
  return EXIT_SUCCESS;
}
