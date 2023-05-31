#include <stdlib.h>
#include <stdio.h>

void show_file(char *filename) {
  // open the file
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "cat: cannot open %s\n", filename);
    exit(1);
  }

  // Now read the contents of the open file and print them out
  int ch;
  while ((ch = getc(fp)) != EOF) {
    putchar(ch);
    printf("\n");
  }
  // close the file
  fclose(fp);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: cat filename!\n");
    exit(1);
  }
  show_file(argv[1]);
  return 0;
}
