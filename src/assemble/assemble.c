#include <stdlib.h>
#include "string.h"
#include <stdio.h>
#include "decoder/decoder.h"
#include "io/io.h"
#include <inttypes.h>

static void print_binary(void *input) {
  uint32_t number = *(uint32_t *) input;
  printf("%"PRIu32, number);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Invalid number of arguments!");
    return 1;
  }
  //char line[] = "ldr x20, [x5] #8"; // post-index test
  //char line[] = "ldr x20, [x5, #8]!"; // pre-index test
  //char line[] = "ldr x20, [x5, #8]"; // unsigned offset test
  //char line[] = "ldr x20, [x5, x8]"; // reg test
  //char line1[] = "foo";
  //char line2[] = "ldr x0, foo";
  ArrayList *lines = create_ArrayList(NULL, free);
  //add_ArrayList_element(lines, line);
  read_file(argv[1], lines);
  ArrayList *tokenized_lines = tokenize(lines);
  free_ArrayList(lines);
  PARSE(tokenized_lines);
  ArrayList *binaryLines = create_ArrayList(print_binary, free);
  for (int i = 0; i < trees->size; i++) {
    ParserTree *tree = get_ArrayList_element(trees, i);
    uint32_t *outputVal = decoder(tree);
    add_ArrayList_element(binaryLines, outputVal);
  }
  free_ArrayList(trees);
  print_ArrayList_elements(binaryLines); // prints instructions to stdout
  write_binary(binaryLines, argv[2]); //prints instructions to bin file
  free_ArrayList(binaryLines);
  return EXIT_SUCCESS;
}
