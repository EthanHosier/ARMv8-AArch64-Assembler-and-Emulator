#ifndef IO_H
#define IO_H
#include "../../ArrayList.h"

extern void write_binary(ArrayList *binaryLines, char *fileName);

extern void read_file(char *fileName, ArrayList *lines);

#endif