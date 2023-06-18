#include "assemble.h"
#include <stdlib.h>
#include "string.h"
#include <stdio.h>
#include "decoder/decoder.h"

int main(int argc, char **argv) {
  //char line[] = "ldr x20, [x5] #8"; // post-index test
  //char line[] = "ldr x20, [x5, #8]!"; // pre-index test
  //char line[] = "ldr x20, [x5, #8]"; // unsigned offset test
  //char line[] = "ldr x20, [x5, x8]"; // reg test
  //char line1[] = "foo";
  //char line2[] = "ldr x0, foo";
  ArrayList *lines = create_ArrayList(NULL, NULL);
  //add_ArrayList_element(lines, line);
  FILE *fileIn = fopen(argv[1], "r");
  if (fileIn == NULL) {
    printf("Failed to open the file.\n");
    return EXIT_FAILURE;
  }
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), fileIn) != NULL) {
    add_ArrayList_element(lines, buffer); // read from files
  }
  fclose(fileIn);
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
  FILE *fileOut = fopen(argv[2], "w");
  if (fileOut == NULL) {
    printf("Failed to open the file.\n");
    return EXIT_FAILURE;
  }
  for (int i = 0; i < binaryLines->size; i++) {
    for (int j = 0; j < 32; j++) {
      fprintf(fileOut, "%b", *((uint32_t *) get_ArrayList_element(binaryLines, i)));
    }
  }
  free_ArrayList(trees);
  return EXIT_SUCCESS;
}
