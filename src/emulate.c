#include <stdio.h>
#include "utils.h"
#include <inttypes.h>

#define MAX_WORDS 100
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: cat filename!\n");
    return 1;
  }
  uint32_t instructions[MAX_WORDS];

  readBinaryFile(argv[1], instructions, MAX_WORDS);

  // Print the instructions
  /* for (size_t i = 0; i < 3; i++) {
    fprintf(stdout, "Word %zu: %" PRIu32 "\n", i + 1, instructions[i]);
  } */
  SystemState state;
  initialiseSystemState(state);
  printf("Hello world");
  return 0;
}
