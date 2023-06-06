#include "system.h"
#include "../io/io.h"
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

static void zero64Array(uint64_t array[], int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

static void zero32Array(uint32_t array[], int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

static void zero8Array(uint8_t array[], int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

static int invalidInstruction(void) {
  fprintf(stderr, "Invalid instruction!");
  return 1;
}

static void updateBitsSubset(bool bits[], int newBits, int msb, int lsb) {
  for (int i = lsb; i <= msb; i++) {
    bits[i] = newBits & 1;
    newBits = newBits >> 1;
  }
}

static int32_t getBitsSubsetSigned(const bool bits[], int msb, int lsb) {
  uint32_t subset = bits[msb] ? -1 : 0;
  for (int i = msb; i >= lsb; i--) {
    subset = (subset << 1) | bits[i];
  }
  return (int32_t) subset;
}

static int getMemAddress(bool bits[]) {
  int xn = getBitsSubsetSigned(bits, 9, 5);
  if (bits[21] && !bits[15] && bits[14] && bits[13] && !bits[12] && bits[11] &&
      !bits[10]) {
    //register offset
    int xm = getBitsSubsetSigned(bits, 20, 16);
    return xn + xm;
  } else if (!bits[21] && bits[10]) {
    //Pre/Post Index
    //INCORRECT IMPLEMENTATION (don't understand)
    int simm9 = getBitsSubsetSigned(bits, 20, 12);
    updateBitsSubset(bits, xn + simm9, 9, 5);
    if (bits[11]) {
      return xn + simm9;
    } else {
      return xn;
    }
  } else {
    //Unsigned Offset
    int imm12 = getBitsSubsetSigned(bits, 21, 10);
    return xn + imm12;
  }
}

static uint32_t getBitsSubsetUnsigned(const bool bits[], int msb, int lsb) {
  uint32_t subset = 0;
  for (int i = msb; i >= lsb; i--) {
    subset = (subset << 1) | bits[i];
  }
  return subset;
}

static int32_t
convertFromUnsignedToSigned(const bool bits[], uint32_t number, int posOfMSB) {
  int32_t signedNumber = (int32_t) number;
  return (int32_t) (bits[posOfMSB]) ? -signedNumber : signedNumber;
}

static uint64_t zeroPad32BitSigned(int32_t num) {
  return (uint64_t) ((uint32_t) num);
}

static int checkOverUnderflow32(int32_t a, int32_t b) {
  if ((b > 0 && a > INT32_MAX - b) ||
      (b < 0 && a < INT32_MIN - b) ||
      (b > 0 && a < INT32_MIN + b) ||
      (b < 0 && a > INT32_MAX - b)) {
    return 1;
  } else {
    return 0;
  }
}

static int checkOverUnderflow64(int64_t a, int64_t b) {
  if ((b > 0 && a > INT64_MAX - b) ||
      (b < 0 && a < INT64_MIN - b) ||
      (b > 0 && a < INT64_MIN + b) ||
      (b < 0 && a > INT64_MAX - b)) {
    return 1;
  } else {
    return 0;
  }
}

//TODO: add carry flag + pls test this
//assume the number given is a 64 bit
static uint64_t asr64(uint64_t operand, int bitsToShift) {
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
static uint32_t asr32(uint32_t operand, int bitsToShift) {
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
static uint64_t ror64(uint64_t operand, int bitsToRotate) {
  assert(bitsToRotate < 64);

  uint64_t ones = (UINT64_C(1) << bitsToRotate) - UINT64_C(1);
  uint64_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (64 - bitsToRotate));
  return operand;
}

//test
//assume the number given is the 32 bit used portion of the operand
static uint32_t ror32(uint32_t operand, int bitsToRotate) {
  assert(bitsToRotate < 32);

  uint32_t ones = (UINT32_C(1) << bitsToRotate) - UINT32_C(1);
  uint32_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (32 - bitsToRotate));
  return operand;
}

static uint32_t
conditionalShiftForLogical32(uint32_t shiftCond, uint32_t valToShift,
                             int shiftMagnitude) {
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

static uint64_t
conditionalShiftForLogical64(uint64_t shiftCond, uint64_t valToShift,
                             int shiftMagnitude) {
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

static void b(SystemState *state, const bool bits[]) {
  int64_t simm26 = (int64_t) getBitsSubsetSigned(bits, 25, 0);
  (*state).programCounter += simm26;
}

static void br(SystemState *state, const bool bits[]) {
  (*state).programCounter = (*state).generalPurpose[getBitsSubsetUnsigned(bits,
                                                                          9,
                                                                          5)];
}

static void beq(SystemState *state, const bool bits[]) {
  if ((*state).pState.zero) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

static void bne(SystemState *state, const bool bits[]) {
  if (!(*state).pState.zero) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

static void bge(SystemState *state, const bool bits[]) {
  if ((*state).pState.negative == (*state).pState.overflow) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

static void blt(SystemState *state, const bool bits[]) {
  if ((*state).pState.negative != (*state).pState.overflow) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

static void bgt(SystemState *state, const bool bits[]) {
  if (!(*state).pState.zero &&
      (*state).pState.negative == (*state).pState.overflow) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

static void ble(SystemState *state, const bool bits[]) {
  if (!(!(*state).pState.zero &&
        (*state).pState.negative == (*state).pState.overflow)) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  }
}

static void bal(SystemState *state, const bool bits[]) {
  int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
  (*state).programCounter += simm19;
}

static int32_t read32bitReg(SystemState *state, uint32_t reg) {
  return (int32_t) (*state).generalPurpose[reg];
}

static int64_t read64bitReg(SystemState *state, uint32_t reg) {
  return (int64_t) (*state).generalPurpose[reg];
}

static void write32bitReg(SystemState *state, uint32_t reg, uint32_t value) {
  (*state).generalPurpose[reg] = value;
}

static void write64bitReg(SystemState *state, uint32_t reg, uint64_t value) {
  (*state).generalPurpose[reg] = value;
}

static void and64_bic64(SystemState *state, uint64_t rd_reg, int64_t rn_dat,
                        int64_t rm_dat) {
  write64bitReg(state, rd_reg, rn_dat & rm_dat);
}

static void orr64_orn64(SystemState *state, uint64_t rd_reg, int64_t rn_dat,
                        int64_t rm_dat) {
  write64bitReg(state, rd_reg, rn_dat | rm_dat);
}

static void eor64_eon64(SystemState *state, uint64_t rd_reg, int64_t rn_dat,
                        int64_t rm_dat) {
  write64bitReg(state, rd_reg, rn_dat ^ rm_dat);
}

static void ands64_bics64(SystemState *state, uint64_t rd_reg, int64_t rn_dat,
                          int64_t rm_dat) {
  int64_t result = rn_dat & rm_dat;
  write64bitReg(state, rd_reg, result);
  (*state).pState.negative = result < 0;
  (*state).pState.zero = result == 0;
  (*state).pState.carry = 0;
  (*state).pState.overflow = 0;
}

static void and32_bic32(SystemState *state, uint32_t rd_reg, int32_t rn_dat,
                        int32_t rm_dat) {
  write32bitReg(state, rd_reg, rn_dat & rm_dat);
}

static void orr32_orn32(SystemState *state, uint64_t rd_reg, int32_t rn_dat,
                        int32_t rm_dat) {
  write32bitReg(state, rd_reg, rn_dat | rm_dat);
}

static void eor32_eon32(SystemState *state, uint64_t rd_reg, int32_t rn_dat,
                        int32_t rm_dat) {
  write32bitReg(state, rd_reg, rn_dat ^ rm_dat);
}

static void ands32_bics32(SystemState *state, uint64_t rd_reg, int32_t rn_dat,
                          int32_t rm_dat) {
  int64_t result = rn_dat & rm_dat;
  write32bitReg(state, rd_reg, result);
  (*state).pState.negative = result < 0;
  (*state).pState.zero = result == 0;
  (*state).pState.carry = 0;
  (*state).pState.overflow = 0;
}

static int executeImmediateDP(SystemState *state, const bool bits[]) {
  uint32_t opi = getBitsSubsetUnsigned(bits, 25, 23);
  uint32_t opc = getBitsSubsetUnsigned(bits, 30, 29);
  bool sf = bits[31];
  uint32_t rd = getBitsSubsetUnsigned(bits, 4, 0);
  assert(rd < 32);
  switch (opi) {
    case 2://opi = 010 (ARITHMETIC)
    {
      bool sh = bits[22];
      uint32_t rn = getBitsSubsetUnsigned(bits, 9, 5);
      assert(rn < 32);
      int32_t imm12 = getBitsSubsetSigned(bits, 21, 10);
      if (sh) {
        imm12 = imm12 << 12;
      }
      switch (opc) {
        case 0://opc = 00 (add)
          if (sf) {
            (*state).generalPurpose[rd] =
                    ((int64_t) (*state).generalPurpose[rn]) + imm12;
          } else {
            (*state).generalPurpose[rd] = zeroPad32BitSigned(
                    ((int32_t) (*state).generalPurpose[rn]) + imm12);
          }
          break;
        case 1://opc = 01 (adds)
          if (sf) {
            int64_t res = (int64_t) ((*state).generalPurpose[rn]) + imm12;
            (*state).generalPurpose[rd] = res;
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow64(
                    (int64_t) (*state).generalPurpose[rn],
                    (int64_t) imm12);
          } else {
            int32_t res = (int32_t) ((*state).generalPurpose[rn]) + imm12;
            (*state).generalPurpose[rd] = zeroPad32BitSigned(res);
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow32(
                    (int32_t) ((*state).generalPurpose[rn]), imm12);
          }
          break;
        case 2://opc = 10 (sub)
          if (sf) {
            (*state).generalPurpose[rd] =
                    ((int64_t) (*state).generalPurpose[rn]) - imm12;
          } else {
            (*state).generalPurpose[rd] = zeroPad32BitSigned(
                    ((int32_t) (*state).generalPurpose[rn]) - imm12);
          }
          break;
        case 3://opc = 11 (subs)
          if (sf) {
            int64_t res = (int64_t) ((*state).generalPurpose[rn]) - imm12;
            (*state).generalPurpose[rd] = res;
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow64(
                    (int64_t) (*state).generalPurpose[rn],
                    (int64_t) imm12);
          } else {
            int32_t res = (int32_t) ((*state).generalPurpose[rn]) - imm12;
            (*state).generalPurpose[rd] = zeroPad32BitSigned(res);
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow32(
                    (int32_t) ((*state).generalPurpose[rn]), imm12);
          }
          break;
        default:
          return invalidInstruction();
      }
      break;
    }
    case 5://opi = 101
    {
      uint32_t hw = getBitsSubsetUnsigned(bits, 22, 21);
      int16_t imm16 = (int16_t) getBitsSubsetSigned(bits, 20,
                                                    5); //assuming this number is meant to be signed

      int32_t shift = (int32_t) (hw * 16);
      int64_t op64 = imm16 << shift;
      int32_t op32 = imm16 << shift;

      switch (opc) {
        case 0://opc = 00 (movn)
          if (sf) //64 bit
          {
            (*state).generalPurpose[rd] = ~op64;
          } else {
            (*state).generalPurpose[rd] = ~op32;
          }

          break;
        case 2://opc = 10 (movz)
          if (sf) {
            (*state).generalPurpose[rd] = op64;
          } else {
            (*state).generalPurpose[rd] = op32;
          }

          break;
        case 3://opc = 11 (movk)
          //TODO: add when 11111 case (for entire instruction set)
          //TESTTTTTTTTTTTTTTTTT
          if (sf) {//64 bit
            uint64_t val = (*state).generalPurpose[rd];
            uint64_t top = val / (1 << (shift + 15)); //might be +14 idk
            uint64_t bottom = val % (1 << (shift - 1)); //i think -1

            uint64_t joined = (top << (shift + 15)) |
                              (((uint64_t) imm16) << (shift - 1)) | bottom;
            (*state).generalPurpose[rd] = joined;
          } else {
            uint32_t val = (uint32_t) (*state).generalPurpose[rd];
            uint32_t top = val / (1 << (shift + 15)); //might be +14 idk
            uint32_t bottom = val % (1 << (shift - 1)); //i think -1

            uint64_t joined = (uint64_t) ((top << (shift + 15)) |
                                          (((uint64_t) imm16) << (shift - 1)) |
                                          bottom);
            (*state).generalPurpose[rd] = joined;
          }
          break;
        default:
          return invalidInstruction();
      }
    }
      break;
    default:
      return invalidInstruction();
  }
  fprintf(stdout, "Immediate DP Instruction\n");
  (*state).programCounter++;
  return 0;
}

static int executeRegisterDP(SystemState *state, const bool bits[]) {
  /*
   * "and x0 x0 x0" is the halt instruction - use premature `return` statement
   * after overriding the value of the PC. This will avoid the PC being
   * incremented at the end of this function.
  */
  /* ^ Halt instruction now implemented! */
  uint32_t m_opr = (bits[28] << 4) | getBitsSubsetUnsigned(bits, 24, 21);
  uint32_t opc = getBitsSubsetUnsigned(bits, 30, 29);
  uint32_t opc_n = (opc << 1) | bits[21];
  uint32_t opc_x = (opc << 1) | bits[15];
  bool sf = bits[31];

  uint32_t rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
  uint32_t shift = getBitsSubsetUnsigned(bits, 23, 22);
  int32_t operand = getBitsSubsetSigned(bits, 15, 10);
  int64_t rn_dat = (int64_t) (*state).generalPurpose[getBitsSubsetUnsigned(bits,
                                                                           9,
                                                                           5)];
  int64_t rm_dat = (int64_t) (*state).generalPurpose[getBitsSubsetUnsigned(bits,
                                                                           20,
                                                                           16)];
  switch (m_opr) {
    case 8:
    case 10:
    case 12:
    case 14://M = 0, opr = 1xx0 (fall-through for unknowns)
      switch (opc) {
        case 0://opc = 00 (add)
          if (sf) {
            rm_dat = (int64_t) conditionalShiftForLogical64(shift, rm_dat,
                                                            operand);
            (*state).generalPurpose[rd_reg] =
                    (int64_t) rn_dat + (int64_t) rm_dat;
          } else {
            rm_dat = conditionalShiftForLogical32(shift, (uint32_t) rm_dat,
                                                  operand);
            (*state).generalPurpose[rd_reg] = zeroPad32BitSigned(
                    (int32_t) rn_dat + (int32_t) rm_dat);
          }
          break;
        case 1://opc = 01 (adds)
          if (sf) {
            rm_dat = (int64_t) conditionalShiftForLogical64(shift, rm_dat,
                                                            operand);
            int64_t res = (int64_t) rn_dat + (int64_t) rm_dat;
            (*state).generalPurpose[rd_reg] = res;
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow64((int64_t) rn_dat,
                                                            (int64_t) rm_dat);
          } else {
            rm_dat = conditionalShiftForLogical32(shift, rm_dat, operand);
            int32_t res = (int32_t) rn_dat + (int32_t) rm_dat;
            (*state).generalPurpose[rd_reg] = zeroPad32BitSigned(res);
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow32((int32_t) rn_dat,
                                                            (int32_t) rm_dat);
          }
          break;
        case 2://opc = 10 (sub)
          if (sf) {
            rm_dat = (int64_t) conditionalShiftForLogical64(shift, rm_dat,
                                                            operand);
            (*state).generalPurpose[rd_reg] =
                    (int64_t) rn_dat - (int64_t) rm_dat;
          } else {
            rm_dat = conditionalShiftForLogical32(shift, (uint32_t) rm_dat,
                                                  operand);
            (*state).generalPurpose[rd_reg] = zeroPad32BitSigned(
                    (int32_t) rn_dat - (int32_t) rm_dat);
          }
          break;
        case 3://opc  = 11 (subs)
          if (sf) {
            rm_dat = (int64_t) conditionalShiftForLogical64(shift, rm_dat,
                                                            operand);
            int64_t res = (int64_t) rn_dat - (int64_t) rm_dat;
            (*state).generalPurpose[rd_reg] = res;
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow64((int64_t) rn_dat,
                                                            (int64_t) rm_dat);
          } else {
            rm_dat = conditionalShiftForLogical32(shift, rm_dat, operand);
            int32_t res = (int32_t) rn_dat - (int32_t) rm_dat;
            (*state).generalPurpose[rd_reg] = zeroPad32BitSigned(res);
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow32((int32_t) rn_dat,
                                                            (int32_t) rm_dat);
          }
          break;
        default:
          return invalidInstruction();
      }
      break;
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7://M = 0, opr = 0xxx (fall-through for unknowns)

      sf = bits[31];
      if (sf) {
        rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        uint32_t rn_reg = getBitsSubsetUnsigned(bits, 9, 5);
        uint32_t rm_reg = getBitsSubsetUnsigned(bits, 20, 16);
        rn_dat = read64bitReg(state, rn_reg);
        rm_dat = read64bitReg(state, rm_reg);
        shift = getBitsSubsetUnsigned(bits, 23, 22);
        operand = getBitsSubsetSigned(bits, 15, 10);
        rm_dat = conditionalShiftForLogical32(shift, rm_dat, operand);
        switch (opc_n) {
          case 0://opc = 00, N = 0 (and)
            if (rm_reg == 0 && rd_reg == 0 && rn_reg == 0) {
              return HALT;
            }
            and64_bic64(state, rd_reg, rn_dat, rm_dat);
            break;
          case 1://opc = 00, N = 1 (bic)
            //bic
            and64_bic64(state, rd_reg, rn_dat, ~rm_dat);
            break;
          case 2://opc = 01, N = 0 (orr)
            //orr
            orr64_orn64(state, rd_reg, rn_dat, rm_dat);
            break;
          case 3://opc = 01, N = 1 (orn)
            //orn
            orr64_orn64(state, rd_reg, rn_dat, ~rm_dat);
            break;
          case 4://opc = 10, N = 0 (eor)
            //eor
            eor64_eon64(state, rd_reg, rn_dat, rm_dat);
            break;
          case 5://opc = 10, N = 1 (eon)
            //eon
            eor64_eon64(state, rd_reg, rn_dat, ~rm_dat);
            break;
          case 6://opc = 11, N = 0 (ands)
            //ands
            ands64_bics64(state, rd_reg, rn_dat, rm_dat);
            break;
          case 7://opc = 11, N = 1 (bics)
            //bics
            ands64_bics64(state, rd_reg, rn_dat, ~rm_dat);
            break;
          default:
            return invalidInstruction();
        }
      } else {
        rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        int32_t rn_dat_32 = read32bitReg(state,
                                         getBitsSubsetUnsigned(bits, 9, 5));
        int32_t rm_dat_32 = read32bitReg(state,
                                         getBitsSubsetUnsigned(bits, 20, 16));
        shift = getBitsSubsetUnsigned(bits, 23, 22);
        operand = getBitsSubsetSigned(bits, 15, 10);
        rm_dat_32 = (int32_t) conditionalShiftForLogical32(shift, rm_dat_32,
                                                           operand);
        switch (opc_n) {
          case 0://opc = 00, N = 0 (and)
            and32_bic32(state, rd_reg, rn_dat_32, rm_dat_32);
            break;
          case 1://opc = 00, N = 1 (bic)
            //bic
            and32_bic32(state, rd_reg, rn_dat_32, ~rm_dat_32);
            break;
          case 2://opc = 01, N = 0 (orr)
            //orr
            orr32_orn32(state, rd_reg, rn_dat_32, rm_dat_32);
            break;
          case 3://opc = 01, N = 1 (orn)
            //orn
            orr32_orn32(state, rd_reg, rn_dat_32, ~rm_dat_32);
            break;
          case 4://opc = 10, N = 0 (eor)
            //eor
            eor32_eon32(state, rd_reg, rn_dat_32, rm_dat_32);
            break;
          case 5://opc = 10, N = 1 (eon)
            //eon
            eor32_eon32(state, rd_reg, rn_dat_32, ~rm_dat_32);
            break;
          case 6://opc = 11, N = 0 (ands)
            //ands
            ands32_bics32(state, rd_reg, rn_dat_32, rm_dat_32);
            break;
          case 7://opc = 11, N = 1 (bics)
            //bics
            ands32_bics32(state, rd_reg, rn_dat_32, ~rm_dat_32);
            break;
          default:
            return invalidInstruction();
        }
      }
      break;
    case 24://M = 1, opr = 1000
      if (bits[31]) {
        rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        rn_dat = read64bitReg(state, getBitsSubsetUnsigned(bits, 9, 5));
        int64_t ra_dat = read64bitReg(state,
                                      getBitsSubsetUnsigned(bits, 14, 10));
        rm_dat = read64bitReg(state, getBitsSubsetUnsigned(bits, 20, 16));
        switch (opc_x) {
          case 0://opc = 00, x = 0 (madd)
            //madd
            write64bitReg(state, rd_reg, ra_dat + (rn_dat * rm_dat));
            break;
          case 1://opc = 00, x = 1 (msub)
            //msub
            write64bitReg(state, rd_reg, ra_dat - (rn_dat * rm_dat));
            break;
          default:
            return invalidInstruction();
        }
      } else {
        rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        rn_dat = read32bitReg(state, getBitsSubsetUnsigned(bits, 9, 5));
        int32_t ra_dat = read32bitReg(state,
                                      getBitsSubsetUnsigned(bits, 14, 10));
        rm_dat = read32bitReg(state, getBitsSubsetUnsigned(bits, 20, 16));
        switch (opc_x) {
          case 0://opc = 00, x = 0 (madd)
            //madd
            write32bitReg(state, rd_reg, ra_dat + (rn_dat * rm_dat));
            break;
          case 1://opc = 00, x = 1 (msub)
            //msub
            write32bitReg(state, rd_reg, ra_dat - (rn_dat * rm_dat));
            break;
          default:
            return invalidInstruction();
        }
      }
      break;
    default:
      return invalidInstruction();
  }
  fprintf(stdout, "Register DP Instruction\n");
  (*state).programCounter++;
  return 0;
}

static int executeSingleDataTransfer(SystemState *state, bool bits[]) {
  uint32_t rt = getBitsSubsetUnsigned(bits, 4, 0);
  if (bits[30]) {//64bit

    if (bits[22]) {//load

      uint64_t val = 0;
      int base = getMemAddress(bits);
      for (int i = 0; i < 8; i++) {
        val = val | (*state).dataMemory[base + i] << i * 8;
      }
      (*state).generalPurpose[rt] = val;

    } else {//store

      int base = getMemAddress(bits);
      uint64_t val = (*state).generalPurpose[rt];
      for (int i = 0; i < 8; i++) {
        unsigned int mask = (1 << ((8 * i + 7) - (8 * i) + 1)) -
                            1;//mask of 1s with correct size
        mask = mask << i;  //shift mask to correct pos
        (*state).dataMemory[base + i] = (val & mask) >> i;
      }
      (*state).dataMemory[getMemAddress(bits)] = (*state).generalPurpose[rt];
    }

  } else {//32bit

    if (bits[22]) {//load
      uint64_t val = 0;
      int base = getMemAddress(bits);
      for (int i = 0; i < 4; i++) {
        val = val | (*state).dataMemory[base + i] << i * 8;
      }
      (*state).generalPurpose[rt] = val;

    } else {//store

      int base = getMemAddress(bits);
      uint64_t val = (*state).generalPurpose[rt];
      for (int i = 0; i < 8; i++) {
        unsigned int mask = (1 << ((8 * i + 7) - (8 * i) + 1)) -
                            1;//mask of 1s with correct size
        mask = mask << i;  //shift mask to correct pos
        (*state).dataMemory[base + i] = (val & mask) >> i;
      }
      (*state).dataMemory[getMemAddress(bits)] = (*state).generalPurpose[rt];
    }

  }


  fprintf(stdout, "Single Data Transfer Instruction\n");
  (*state).programCounter++;
  return 0;
}

static int executeLoadLiteral(SystemState *state, bool bits[]) {
  uint32_t rt = getBitsSubsetUnsigned(bits, 4, 0);
  int32_t simm19 = getBitsSubsetSigned(bits, 23, 5);
  int32_t address = (int32_t) ((*state).programCounter + simm19);

  if (bits[30]) {//64bit

    uint64_t val = 0;
    int base = address;
    for (int i = 0; i < 8; i++) {
      val = val | (*state).dataMemory[base + i] << i * 8;
    }
    (*state).generalPurpose[rt] = val;

  } else {//32bit

    uint64_t val = 0;
    int base = getMemAddress(bits);
    for (int i = 0; i < 4; i++) {
      val = val | (*state).dataMemory[base + i] << i * 8;
    }
    (*state).generalPurpose[rt] = val;

  }

  fprintf(stdout, "Load Literal Instruction\n");
  (*state).programCounter++;
  return 0;
}

static int executeBranch(SystemState *state, const bool bits[]) {
  uint32_t valForReg31to10 = getBitsSubsetUnsigned(bits, 31, 10);
  uint32_t valForReg4to0 = getBitsSubsetUnsigned(bits, 4, 0);
  uint32_t valForCond = getBitsSubsetUnsigned(bits, 31, 24);

  if (!bits[31] && !bits[30]) {//b
    b(state, bits);
  } else if (valForReg31to10 == 3508160 && valForReg4to0 == 0) {//br
    br(state, bits);
  } else if (valForCond == 84 && !bits[4]) {//b.cond
    uint32_t cond = getBitsSubsetUnsigned(bits, 3, 0);
    switch (cond) {
      case 0://cond = 0000 (beq)
        beq(state, bits);
        break;
      case 1://cond = 0001 (bne)
        bne(state, bits);
        break;
      case 10://cond = 1010 (bge)
        bge(state, bits);
        break;
      case 11://cond = 1011 (blt)
        blt(state, bits);
        break;
      case 12://cond = 1100 (bgt)
        bgt(state, bits);
        break;
      case 13://cond = 1101 (ble)
        ble(state, bits);
        break;
      case 14://cond = 1110 (bal)
        bal(state, bits);
        break;
      default:
        return invalidInstruction();
    }
  } else {
    return invalidInstruction();
  }

  fprintf(stdout, "Branch Instruction\n");
  (*state).programCounter++;
  return 0;
}

int execute(SystemState *state, bool bits[]) { // Don't forget about `nop` !!
  if (bits[28] && !bits[27] && !bits[26]) { // DP (Immediate)
    return executeImmediateDP(state, bits);
  } else if (bits[27] && !bits[26] && bits[25]) { // DP (Register)
    return executeRegisterDP(state, bits);
  } else if (bits[31] && bits[29] && bits[28] && bits[27] && !bits[26]
             && !bits[25] && bits[24] && !bits[23]) { // Single Data Transfer
    return executeSingleDataTransfer(state, bits);
  } else if (!bits[31] && !bits[29] && bits[28] && bits[27] && !bits[26]
             && !bits[25] && !bits[24]) { // Load Literal
    return executeLoadLiteral(state, bits);
  } else if (!bits[29] && bits[28] && !bits[27] && bits[26]) { // Branch
    return executeBranch(state, bits);
  }
  fprintf(stderr, "Invalid instruction type!");
  return 1;
}

void initialiseSystemState(SystemState *state, int numberOfInstructions,
                           const uint32_t instructions[]) {
  zero64Array((*state).generalPurpose, GENERAL_PURPOSE_REGISTERS);
  zero32Array((*state).instructionMemory, MAX_INSTRUCTIONS);
  for (int i = 0; i < numberOfInstructions; i++) {
    (*state).instructionMemory[i] = instructions[i];
  }
  zero64Array(&(*state).programCounter, 1);
  (*state).pState.negative = false;
  (*state).pState.zero = true;
  (*state).pState.carry = false;
  (*state).pState.overflow = false;
  zero8Array((*state).dataMemory, MEMORY_SIZE_BYTES);
}

static void outputInstruction(FILE *file, uint32_t val) {
  fprintf(file, " : %08"PRIx32"\n", val);
}

void
outputToFile(SystemState *state, char *filename, int numberOfInstructions) {
  FILE *file;
  file = fopen(filename, "w");
  fprintf(file, "Registers:\n");
  for (int i = 0; i < GENERAL_PURPOSE_REGISTERS; i++) {
    fprintf(file, "X%02d    = %016"PRIx64"\n", i, (*state).generalPurpose[i]);
  }
  fprintf(file, "PC     = %016"PRIx64"\nPSTATE : ",
          (*state).programCounter * 4);
  (*state).pState.negative ? fprintf(file, "N") : fprintf(file, "-");
  (*state).pState.zero ? fprintf(file, "Z") : fprintf(file, "-");
  (*state).pState.carry ? fprintf(file, "C") : fprintf(file, "-");
  (*state).pState.overflow ? fprintf(file, "V") : fprintf(file, "-");
  fprintf(file, "\nNon-zero memory:\n0x00000000");
  outputInstruction(file, (*state).instructionMemory[0]);
  for (int i = 1; i < numberOfInstructions; i++) {
    uint32_t val = (*state).instructionMemory[i];
    if (val != 0) {
      fprintf(file, "%#010"PRIx16, (int16_t) (i * 4));
      outputInstruction(file, val);
    }
  }
  for (int i = 0; i < MEMORY_SIZE_BYTES; i++) {
    uint8_t val = (*state).dataMemory[i];
    if (val != 0) {
      fprintf(file, "Data Memory: %#010"PRIx16" : %08"
                    PRIx8"\n", (int16_t) ((i + numberOfInstructions) * 4), val);
    }
  }
  fclose(file);
}
