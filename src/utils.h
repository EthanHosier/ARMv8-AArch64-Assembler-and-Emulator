#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "consts.h"

int readBinaryFile(char *filename, uint32_t *output, int *instructionCount);

typedef struct {
    bool negative;
    bool zero;
    bool carry;
    bool overflow;
} PState;
typedef struct {
    uint64_t generalPurpose[GENERAL_PURPOSE_REGISTERS];
    uint64_t programCounter;
    PState pState;
    uint8_t primaryMemory[MEMORY_SIZE_BYTES];
} SystemState;

void initialiseSystemState(SystemState *state);

void getBits(uint32_t instruction, bool *bits);

void printInstructions(uint32_t instructions[], int numberOfInstructions);

void printInstruction(bool bits[]);

int invalidInstruction(void);

uint64_t ror64(uint64_t, int);

uint32_t ror32(uint32_t, int);

uint64_t asr64(uint64_t, int);

uint32_t asr32(uint32_t, int);

int getMemAddress(bool bits[]);

uint32_t getBitsSubset(const bool bits[], int msb, int lsb);

void updateBitsSubset(bool bits[], int newBits, int msb, int lsb);

int32_t convertFromUnsignedToSigned(bool bits[], uint32_t number, int posOfMSB);

void b(SystemState *state, bool bits[]);

void br(SystemState *state, bool bits[]);

void beq(SystemState *state, bool bits[]);

void bne(SystemState *state, bool bits[]);

void bge(SystemState *state, bool bits[]);

void blt(SystemState *state, bool bits[]);

void bgt(SystemState *state, bool bits[]);

void ble(SystemState *state, bool bits[]);

void bal(SystemState *state, bool bits[]);

int32_t read32bitreg(SystemState *state, uint32_t reg);

int64_t read64bitreg(SystemState *state, uint32_t reg);

void write32bitreg(SystemState *state, uint32_t reg, uint32_t value);

void write64bitreg(SystemState *state, uint32_t reg, uint64_t value);

void and32(SystemState *state, bool bits[]);

void and64(SystemState *state, bool bits[]);

#endif
