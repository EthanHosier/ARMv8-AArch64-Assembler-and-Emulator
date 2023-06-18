#ifndef IO_H
#define IO_H
#include "../../ArrayList.h"

extern void printBinary(ArrayList *binaryLines, char *fileName);

extern void readFileToArray(char *fileName, ArrayList *lines);

#endif