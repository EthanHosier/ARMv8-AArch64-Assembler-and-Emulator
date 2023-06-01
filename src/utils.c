// goofy little utils functions xx
#include <stdio.h>
#include "utils.h"
#include <inttypes.h>
#include <assert.h>

int readBinaryFile(char filename[], uint32_t output[], int *instructionCount) {
  int numWords = 0;
  // Open the binary file
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file \"%s\"!\n", filename);
    return 1;
  }

  // Read instructions from the binary file
  while (fread(&output[numWords], INSTRUCTION_SIZE_BITS / 8, 1, file) == 1) {
    numWords++;
    if (numWords >= MAX_INSTRUCTIONS) {
      fprintf(stderr, "Maximum instruction count reached!\n");
      return 1;
    }
  }
  *instructionCount = numWords;
  fclose(file);
  return 0;
}

static void zero64Array(uint64_t array[], int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

static void zero8Array(uint8_t array[], int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

void initialiseSystemState(SystemState *state) {
  zero64Array((*state).generalPurpose, GENERAL_PURPOSE_REGISTERS);
  zero64Array(&(*state).programCounter, 1);
  (*state).pState.negative = 0;
  (*state).pState.zero = 0;
  (*state).pState.carry = 0;
  (*state).pState.overflow = 0;
  zero8Array((*state).primaryMemory, MEMORY_SIZE_BYTES);
}

void getBits(uint32_t instruction, bool bits[]) {
  for (int i = 0; i < INSTRUCTION_SIZE_BITS; i++) {
    bits[i] = instruction & 1;
    instruction = instruction >> 1;
  }
}

void printInstruction(bool bits[]) {
  for (int i = 0; i < INSTRUCTION_SIZE_BITS; i++) {
    fprintf(stdout, "%i", bits[INSTRUCTION_SIZE_BITS - i - 1]);
    if (i % 4 == 3) {
      fprintf(stdout, " ");
    }
  }
  fprintf(stdout, "\n");
}

void printInstructions(uint32_t instructions[], int numberOfInstructions) {
  for (int i = 0; i < numberOfInstructions; i++) {
    fprintf(stdout, "Instruction %i: %" PRIu32 "\n", i + 1, instructions[i]);
  }
}

int invalidInstruction(void) {
  fprintf(stderr, "Invalid instruction!");
  return 1;
}

//TODO: add carry flag + pls test this
//assume the number given is a 64 bit
uint64_t asr64(uint64_t operand, int bitsToShift) {
  assert(bitsToShift < 64);

  if (bitsToShift == 0) return operand;

  uint64_t ones = (UINT64_C(1) << 63) & operand; //1000000000000000;
  operand = operand >> bitsToShift;

  if (ones != 0) {
    for (int i = 0; i < bitsToShift; i++) {
      ones += (ones >> 1);
    }
    operand = operand | ones;
  }

  return operand;
}

//TODO: add carry flag + pls test this
//assume the number given is 32 bit (the first 32 0s are removed already)
uint32_t asr32(uint32_t operand, int bitsToShift) {
  assert(bitsToShift < 32);

  if (bitsToShift == 0) return operand;

  uint32_t ones = (UINT32_C(1) << 31) & operand;
  operand = operand >> bitsToShift;

  if (ones != 0) {
    for (int i = 0; i < bitsToShift; i++) {
      ones += (ones >> 1);
    }
    operand = operand | ones;
  }

  return operand;
}


//test
uint64_t ror64(uint64_t operand, int bitsToRotate) {
  assert(bitsToRotate <= 64);

  uint64_t ones = (UINT64_C(1) << bitsToRotate) - UINT64_C(1);
  uint64_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (64 - bitsToRotate));
  return operand;
}

//test
//assume the number given is the 32 bit used portion of the operand
uint32_t ror32(uint32_t operand, int bitsToRotate) {
  assert(bitsToRotate <= 32);

  uint32_t ones = (UINT32_C(1) << bitsToRotate) - UINT32_C(1);
  uint32_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (32 - bitsToRotate));
  return operand;
}

int getMemAddress(bool bits[]) {
  uint8_t xn = (uint8_t) getBitsSubset(bits, 9, 5);
  if (bits[21] && !bits[15] && bits[14] && bits[13] && !bits[12] && bits[11] && !bits[10]) {
    //register offset
    uint8_t xm = (uint8_t) getBitsSubset(bits, 20, 16);
    return xn + xm;
  } else if (!bits[21] && bits[10]) {
    //Pre/Post Index
    int16_t simm9 = (int16_t) getBitsSubset(bits, 20, 12);
    updateBitsSubset(bits, xn + simm9, 9, 5);
    if (bits[11]) {
      return xn + simm9;
    } else {
      return xn;
    }
  } else {
    //Unsigned Offset
    uint16_t imm12 = getBitsSubset(bits, 21, 10);
    return xn + imm12;
  }
}

uint16_t getBitsSubset(const bool bits[], int msb, int lsb) {
  uint16_t subset = 0;
  for (int i = msb; i >= lsb; i--) {
    subset = subset << 1 | bits[i];
  }
  return subset;
}

void updateBitsSubset(bool bits[], int newBits, int msb, int lsb) {
  for (int i = lsb; i <= msb; i++) {
    bits[i] = newBits & 1;
    newBits = newBits >> 1;
  }
}