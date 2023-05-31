#include <stdio.h>
#include "utils.h"
#include <inttypes.h>

#define MAX_WORDS 100
int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "Usage: cat filename!\n");
    return 1;
  }

  uint32_t words[MAX_WORDS];

  readBinaryFile(argv[1], words, MAX_WORDS);

  // Print the words
  for (size_t i = 0; i < 3; i++) {
    printf("Word %zu: %" PRIu32 "\n", i + 1, words[i]);
  }

  return 0;
}
