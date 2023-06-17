#include "assemble.h"
#include <stdlib.h>
#include "string.h"

int main(int argc, char **argv) {
  char line[] = "ldr x20, [x5, #8]";
  int number_of_lines = 1;
  char **lines =
      malloc(number_of_lines * sizeof(char *)); //readLines(file_name);
  lines[0] = line;
  ArrayList *token_lines = create_ArrayList(NULL, &free_ArrayList);
  for (int i = 0; i < number_of_lines; i++) {// maybe works
    ArrayList *tokens = tokenize(lines[i]);
    add_ArrayList_element(token_lines, tokens);
    print_ArrayList_elements(tokens);
  }
  free(lines);
  TreeMap *label_identifiers = first_pass(token_lines);
  ArrayList *trees = second_pass(token_lines, label_identifiers);
  free_ArrayList(token_lines);
  free_map(label_identifiers);
  // TODO: Make map of Parser_Tree.type to function pointers that do final step
  for (int i = 0; i < trees->size; i++) {
    //Parser_Tree *tree = get_ArrayList_element(trees, i);

  }
  free_ArrayList(trees);
  return EXIT_SUCCESS;
}


uint32_t buildBinaryDPImm(uint32_t sf,
                          uint32_t opc,
                          uint32_t opi,
                          uint32_t operand,
                          uint32_t rd) {
  return sf << 31
      | opc << 29
      | 1 << 28
      | opi << 23
      | operand << 5
      | rd;
}

uint32_t buildBinaryDPReg(uint32_t sf,
                          uint32_t opc,
                          uint32_t m,
                          uint32_t opr,
                          uint32_t rm,
                          uint32_t operand,
                          uint32_t rn,
                          uint32_t rd) {
  return sf << 31
      | opc << 29
      | m << 28
      | 5 << 26
      | opr << 21
      | rm << 16
      | operand << 10
      | rn << 5
      | rd;
}

uint32_t buildBinarySDT(uint32_t sf,
                        uint32_t u,
                        uint32_t l,
                        uint32_t offset,
                        uint32_t xn,
                        uint32_t rt) {
  return 1 << 31
      | sf << 30
      | 7 << 27
      | u << 24
      | l << 22
      | offset << 10
      | xn << 5
      | rt;
}

uint32_t buildBinaryLoadLiteral(uint32_t sf, uint32_t simm19, uint32_t rt) {
  return sf << 30
      | 3 << 27
      | simm19 << 5
      | rt;
}

uint32_t buildBinaryBranch(void) {
  //TODO
}

uint32_t buildNOP(void) {
  return 0xD503201F;
}
