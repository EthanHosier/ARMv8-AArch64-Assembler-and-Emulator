#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "../consts.h"

extern int
readBinaryFile(char *filename, uint32_t *output, int *instructionCount);

extern void printInstruction(bool bits[]);

#endif
