#include "assemble.h"
#include <stdlib.h>
#include <printf.h>

int main(int argc, char **argv) {
  char line[] = "ldr x20, [x5, x15]";
  ArrayList *tokens = tokenize(line);
  print_ArrayList_elements(tokens);
  return EXIT_SUCCESS;
}
