#include "assemble.h"
#include <stdlib.h>
#include "string.h"
#include <stdio.h>
#include "decoder/decoder.h"
#include "io/io.h"

int main(int argc, char **argv) {
  //char line[] = "ldr x20, [x5] #8"; // post-index test
  //char line[] = "ldr x20, [x5, #8]!"; // pre-index test
  //char line[] = "ldr x20, [x5, #8]"; // unsigned offset test
  //char line[] = "ldr x20, [x5, x8]"; // reg test
  //char line1[] = "foo";
  //char line2[] = "ldr x0, foo";
  ArrayList *lines = create_ArrayList(NULL, NULL);
  //add_ArrayList_element(lines, line);
  readFileToArray(argv[1], lines);
  ArrayList *tokenized_lines = tokenize(lines);
  free_ArrayList(lines);
  PARSE(tokenized_lines);
  // TODO: Make map of Parser_Tree.type to function pointers that do final step
  ArrayList *binaryLines = create_ArrayList(NULL, free);
  for (int i = 0; i < trees->size; i++) {
    Parser_Tree *tree = get_ArrayList_element(trees, i);
    uint32_t *outputVal = decoder(tree);
    add_ArrayList_element(binaryLines, outputVal);
  }
  free_ArrayList(trees);
  printBinary(binaryLines, argv[2]);
  free_ArrayList(trees);
  return EXIT_SUCCESS;
}
