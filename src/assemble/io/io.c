#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

static void printBinaryHelper(uint32_t val, FILE *fileOut) {
  uint64_t mask = 1;
  uint32_t res = 0;
  for (int i = 0; i < 4; i++) {
    mask <<= 8;
    for (int j = 0; j < 8; j++) {
      mask >>= 1;
      res = res | (mask & val);
    }
    mask <<= 8;
  }
  fwrite(&res, sizeof(uint32_t), 1, fileOut);
}

void write_binary(ArrayList *binaryLines, char *fileName) {
  FILE *fileOut = fopen(fileName, "wb");
  if (fileOut == NULL) {
    printf("Failed to open the file.\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < binaryLines->size; i++) {
    uint32_t element = *(uint32_t *) get_ArrayList_element(binaryLines, i);
    printBinaryHelper(element, fileOut);
  }
  fclose(fileOut);
}

static char *skip_whitespace(char *str) {
  while (*str != '\0') {
    if (*str != ' ' && *str != '\n') {
      return str;
    }
    str++;
  }
  return NULL;
}

void read_file(char *fileName, ArrayList *lines) {

  FILE *fileIn = fopen(fileName, "r");
  if (fileIn == NULL) {
    printf("Failed to open the file.\n");
    exit(EXIT_FAILURE);
  }
  char *buffer = malloc(256 * sizeof(char));
  while (fgets(buffer, 256, fileIn) != NULL) {
    char *first_character = skip_whitespace(buffer);
    if (first_character == NULL) continue;
    first_character[strlen(first_character) - 1] = '\0';
    add_ArrayList_element(lines, strdup(first_character)); // read from files
  }
  free(buffer);
  fclose(fileIn);
}
