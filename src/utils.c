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
      operand = operand | ones;
      ones = ones >> 1;
    }
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
      operand = operand | ones;
      ones = ones >> 1;
    }
  }

  return operand;
}


//test
uint64_t ror64(uint64_t operand, int bitsToRotate) {
  assert(bitsToRotate < 64);

  uint64_t ones = (UINT64_C(1) << bitsToRotate) - UINT64_C(1);
  uint64_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (64 - bitsToRotate));
  return operand;
}

//test
//assume the number given is the 32 bit used portion of the operand
uint32_t ror32(uint32_t operand, int bitsToRotate) {
  assert(bitsToRotate < 32);

  uint32_t ones = (UINT32_C(1) << bitsToRotate) - UINT32_C(1);
  uint32_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (32 - bitsToRotate));
  return operand;
}

int getMemAddress(bool bits[]) {
  int xn = getBitsSubset(bits, 9, 5);
  if (bits[21] && !bits[15] && bits[14] && bits[13] && !bits[12] && bits[11] && !bits[10]) {
    //register offset
    int xm = getBitsSubset(bits, 20, 16);
    return xn + xm;
  } else if (!bits[21] && bits[10]) {
    //Pre/Post Index
    int simm9 = getBitsSubsetSigned(bits, 20, 12);
    updateBitsSubset(bits, xn + simm9, 9, 5);
    if (bits[11]) {
      return xn + simm9;
    } else {
      return xn;
    }
  } else {
    //Unsigned Offset
    int imm12 = getBitsSubset(bits, 21, 10);
    return xn + imm12;
  }
}

uint32_t getBitsSubsetUnsigned(const bool bits[], int msb, int lsb) {
  uint32_t subset = 0;
  for (int i = msb; i >= lsb; i--) {
    subset = (subset << 1) | bits[i];
  }
  return subset;
}

int32_t getBitsSubsetSigned(const bool bits[], int msb, int lsb) {
  uint32_t subset = bits[msb] ? -1 : 0;
  for (int i = msb; i >= lsb; i--) {
    subset = (subset << 1) | bits[i];
  }
  return subset;
}

int32_t convertFromUnsignedToSigned(bool bits[], uint32_t number, int posOfMSB) {
  return (int32_t) (bits[posOfMSB]) ? -number : number;
}

void updateBitsSubset(bool bits[], int newBits, int msb, int lsb) {
  for (int i = lsb; i <= msb; i++) {
    bits[i] = newBits & 1;
    newBits = newBits >> 1;
  }
}

void b(SystemState *state, bool bits[]) {
  int64_t simm26 = (int64_t) getBitsSubsetSigned(bits, 25, 0);
  (*state).programCounter += simm26;
}

void br(SystemState *state, bool bits[]) {
  (*state).programCounter = (*state).generalPurpose[getBitsSubset(bits, 9, 5)];
}

void beq(SystemState *state, bool bits[]) {
  if ((*state).pState.zero) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

void bne(SystemState *state, bool bits[]) {
  if (!(*state).pState.zero) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

void bge(SystemState *state, bool bits[]) {
  if ((*state).pState.negative == (*state).pState.overflow) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

void blt(SystemState *state, bool bits[]) {
  if ((*state).pState.negative != (*state).pState.overflow) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

void bgt(SystemState *state, bool bits[]) {
  if (!(*state).pState.zero && (*state).pState.negative == (*state).pState.overflow) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

void ble(SystemState *state, bool bits[]) {
  if (!(!(*state).pState.zero && (*state).pState.negative == (*state).pState.overflow)) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

void bal(SystemState *state, bool bits[]) {
  int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
  (*state).programCounter += simm19;
}

int32_t read32bitreg(SystemState *state, uint32_t reg) {
  return (uint32_t) (*state).generalPurpose[reg];
}

int64_t read64bitreg(SystemState *state, uint32_t reg) {
  return (*state).generalPurpose[reg];
}

void write32bitreg(SystemState *state, uint32_t reg, uint32_t value) {
  (*state).generalPurpose[reg] = value;
}

void write64bitreg(SystemState *state, uint32_t reg, uint64_t value) {
  return (*state).generalPurpose[reg] = value;
}

uint64_t zeroPad32BitSigned(int32_t num) {
    return (uint64_t) ((uint32_t) num);
}

int checkOverUnderflow32(int32_t a, int32_t b) {
  if((b > 0 && a > INT32_MAX - b) ||
  (b < 0 && a < INT32_MIN - b) ||
  (b > 0 && a < INT32_MIN + b) ||
  (b < 0 && a > INT32_MAX - b)) {
    return 1;
  } else {
    return 0;
  }
}

int checkOverUnderflow64(int64_t a, int64_t b) {
  if((b > 0 && a > INT64_MAX - b) ||
  (b < 0 && a < INT64_MIN - b) ||
  (b > 0 && a < INT64_MIN + b) ||
  (b < 0 && a > INT64_MAX - b)) {
    return 1;
  } else {
    return 0;
  }
}

uint32_t conditionalShiftForLogical32(uint32_t shiftCond, uint32_t valToShift, uint32_t shiftMagnitude) {
  switch (shiftCond) {
    case 0://shift = 00
      return valToShift << shiftMagnitude;
    case 1://shift = 01
      return valToShift >> shiftMagnitude;
    case 2://shift = 10
      return asr64(valToShift, shiftMagnitude);
    default://shift = 11
      return ror64(valToShift, shiftMagnitude);
  }
}

uint64_t conditionalShiftForLogical64(uint64_t shiftCond, uint64_t valToShift, uint64_t shiftMagnitude) {
  switch (shiftCond) {
    case 0://shift = 00
      return valToShift << shiftMagnitude;
    case 1://shift = 01
      return valToShift >> shiftMagnitude;
    case 2://shift = 10
      return asr64(valToShift, shiftMagnitude);
    default://shift = 11
      return ror64(valToShift, shiftMagnitude);
  }
}

void and64_bic64(SystemState *state, uint64_t rd_reg, int64_t rn_dat, int64_t rm_dat) {
  write64bitreg(state, rd_reg, rn_dat & rm_dat);
}

void orr64_orn64(SystemState *state, uint64_t rd_reg, int64_t rn_dat, int64_t rm_dat) {
  write64bitreg(state, rd_reg, rn_dat | rm_dat);
}

void eor64_eon64(SystemState *state, uint64_t rd_reg, int64_t rn_dat, int64_t rm_dat) {
  write64bitreg(state, rd_reg, rn_dat ^ rm_dat);
}

void ands64_bics64(SystemState *state, uint64_t rd_reg, int64_t rn_dat, int64_t rm_dat) {
  int64_t result = rn_dat & rm_dat;
  write64bitreg(state, rd_reg, result);
  (*state).pState.negative = result < 0;
  (*state).pState.zero = result == 0;
  (*state).pState.carry = 0;
  (*state).pState.overflow = 0;
}

void and32_bic32(SystemState *state, uint32_t rd_reg, int32_t rn_dat, int32_t rm_dat) {
  write32bitreg(state, rd_reg, rn_dat & rm_dat);
}

void orr32_orn32(SystemState *state, uint64_t rd_reg, int32_t rn_dat, int32_t rm_dat) {
  write32bitreg(state, rd_reg, rn_dat | rm_dat);
}

void eor32_eon32(SystemState *state, uint64_t rd_reg, int32_t rn_dat, int32_t rm_dat) {
  write32bitreg(state, rd_reg, rn_dat ^ rm_dat);
}

void ands32_bics32(SystemState *state, uint64_t rd_reg, int32_t rn_dat, int32_t rm_dat) {
  int64_t result = rn_dat & rm_dat;
  write32bitreg(state, rd_reg, result);
  (*state).pState.negative = result < 0;
  (*state).pState.zero = result == 0;
  (*state).pState.carry = 0;
  (*state).pState.overflow = 0;
}

void outputToFile(SystemState *state) {
  FILE *file;
  file = fopen("output.out", "w");

  fprintf(file, "Registers:\n");
  for (int i = 0; i < GENERAL_PURPOSE_REGISTERS; i ++) {
    fprintf(file, "X%d = %x", i, (*state).generalPurpose[i]);
  }
  fprintf(file, "PC = %x\n", (*state).programCounter*4);
  fprintf(file, "PSTATE : ");
  (*state).pState.negative ? fprintf(file, "N") : fprintf(file, "-");
  (*state).pState.zero ? fprintf(file, "Z") : fprintf(file, "-");
  (*state).pState.carry ? fprintf(file, "C") : fprintf(file, "-");
  (*state).pState.overflow ? fprintf(file, "V") : fprintf(file, "-");
  fprintf(file, "\nNon-zero memory:\n");
  for (int i = 0; i < MEMORY_SIZE_BYTES; i ++) {
    uint8_t val = (*state).primaryMemory[i];
    if (val != 0) {
      fprintf(file, "%x: %x", i*4, val);
    }
  }

  fclose(file);
}