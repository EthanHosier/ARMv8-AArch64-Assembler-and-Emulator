#include "system.h"
#include "../io/io.h"
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

#define zeroArray(array, size)     \
  for (int i = 0; i < (size); i++) \
    (array)[i] = 0;

#define getBitsSubset                 \
  for (int i = msb; i >= lsb; i--)    \
    subset = (subset << 1) | bits[i];

#define asr_general                         \
  if (ones != 0) {                          \
    for (int i = 0; i < bitsToShift; i++) { \
      operand = operand | ones;             \
      ones = ones >> 1;                     \
    }                                       \
  }

#define ror_general(bits)                        \
  operand = operand >> bitsToRotate;             \
  operand += (toAdd << ((bits) - bitsToRotate));

#define checkOverflow(bits)             \
  (b > 0 && a > INT##bits##_MAX - b) || \
  (b < 0 && a < INT##bits##_MIN - b) || \
  (b > 0 && a < INT##bits##_MIN + b) || \
  (b < 0 && a > INT##bits##_MAX - b)

#define conditionalShift(bits)                       \
  switch (shiftCond) {                               \
    case 0:                                          \
      return valToShift << shiftMagnitude;           \
    case 1:                                          \
      return valToShift >> shiftMagnitude;           \
    case 2:                                          \
      return asr##bits(valToShift, shiftMagnitude);  \
    default:                                         \
      return ror##bits(valToShift, shiftMagnitude);  \
  }

#define addsImmediateDP(bits)                                                \
  int##bits##_t res = (int##bits##_t) ((*state).generalPurpose[rn]) + imm12; \
    if (rd != 31) {                                                          \
      (*state).generalPurpose[rd] = res;                                     \
    }                                                                        \
  (*state).pState.negative = res < 0;                                        \
  (*state).pState.zero = res == 0;                                           \
  (*state).pState.carry = 0;                                                 \
  (*state).pState.overflow = checkOverUnderflow##bits(                       \
      (int##bits##_t) (*state).generalPurpose[rn],                           \
      (int##bits##_t) imm12);

#define subsImmediateDP(bits)                                            \
  int##bits##_t minuend = (int##bits##_t) ((*state).generalPurpose[rn]); \
  int##bits##_t subtrahend = (int##bits##_t) imm12;                      \
  int##bits##_t res = minuend - subtrahend;                              \
  if (rd != 31) {                                                        \
    (*state).generalPurpose[rd] = (uint##bits##_t) res;                                    \
  }                                                                      \
  (*state).pState.negative = res < 0;                                    \
  (*state).pState.zero = res == 0;                                       \
  (*state).pState.carry = minuend >= subtrahend;                         \
  (*state).pState.overflow = checkOverUnderflow##bits(                   \
      (int##bits##_t) ((*state).generalPurpose[rn]), subtrahend);

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

static uint32_t getBitsSubsetUnsigned(const bool bits[], int msb, int lsb) {
  uint32_t subset = 0;
  getBitsSubset
  return subset;
}

static int32_t getBitsSubsetSigned(const bool bits[], int msb, int lsb) {
  uint32_t subset = bits[msb] ? -1 : 0;
  getBitsSubset
  return (int32_t) subset;
}

static uint32_t getMemAddress(SystemState *state, bool bits[]) {
  uint32_t xn = getBitsSubsetUnsigned(bits, 9, 5);
  if (bits[24]) {
    //Unsigned Offset
    uint32_t imm12 = getBitsSubsetUnsigned(bits, 21, 10);
    return (*state).generalPurpose[xn] + ((bits[30]) ? 8 : 4) * imm12;
  } else if (bits[21] && !bits[15] && bits[14] && bits[13] && !bits[12]
      && bits[11] &&
      !bits[10]) {
    //register offset
    uint32_t xm = getBitsSubsetUnsigned(bits, 20, 16);
    return (*state).generalPurpose[xn] + (*state).generalPurpose[xm];
  } else if (!bits[21] && bits[10]) {
    //Pre/Post Index
    int32_t simm9 = getBitsSubsetSigned(bits, 20, 12);
    uint32_t oldVal = (*state).generalPurpose[xn];
    uint32_t newVal = oldVal + simm9;
    (*state).generalPurpose[xn] = newVal;
    if (bits[11]) {
      return newVal;
    } else {
      return oldVal;
    }
  } else {
    fprintf(stderr, "something fucky wucky happened!");
    return 69420;
  }
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
  if (checkOverflow(32)) {
    return 1;
  } else {
    return 0;
  }
}

static int checkOverUnderflow64(int64_t a, int64_t b) {
  if (checkOverflow(64)) {
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
  asr_general
  return operand;
}

//TODO: add carry flag + pls test this
//assume the number given is 32 bit (the first 32 0s are removed already)
static uint32_t asr32(uint32_t operand, int bitsToShift) {
  assert(bitsToShift < 32);
  if (bitsToShift == 0) return operand;
  uint32_t ones = (UINT32_C(1) << 31) & operand;
  operand = operand >> bitsToShift;
  asr_general
  return operand;
}


//test
static uint64_t ror64(uint64_t operand, int bitsToRotate) {
  assert(bitsToRotate < 64);

  uint64_t ones = (UINT64_C(1) << bitsToRotate) - UINT64_C(1);
  uint64_t toAdd = ones & operand;
  ror_general(64)
  return operand;
}

//test
//assume the number given is the 32 bit used portion of the operand
static uint32_t ror32(uint32_t operand, int bitsToRotate) {
  assert(bitsToRotate < 32);
  uint32_t ones = (UINT32_C(1) << bitsToRotate) - UINT32_C(1);
  uint32_t toAdd = ones & operand;
  ror_general(32)
  return operand;
}

static uint32_t
conditionalShiftForLogical32(uint32_t shiftCond, uint32_t valToShift,
                             int shiftMagnitude) {
  conditionalShift(32)
}

static uint64_t
conditionalShiftForLogical64(uint64_t shiftCond, uint64_t valToShift,
                             int shiftMagnitude) {
  conditionalShift(64)
}

static void b(SystemState *state, const bool bits[]) {
  int64_t simm26 = (int64_t) getBitsSubsetSigned(bits, 25, 0);
  (*state).programCounter += simm26;
}

static void br(SystemState *state, const bool bits[]) {
  (*state).programCounter =
      (*state).generalPurpose[getBitsSubsetUnsigned(bits,
                                                    9,
                                                    5)];
}

static void
conditionalBranch(SystemState *state, const bool bits[], bool cond) {
  if (cond) {
    int64_t simm19 = (int64_t) getBitsSubsetSigned(bits, 23, 5);
    (*state).programCounter += simm19;
  } else (*state).programCounter++;
}

static bool beq(SystemState *state) {
  return (*state).pState.zero;
}

static bool bne(SystemState *state) {
  return !beq(state);
}

static bool bge(SystemState *state) {
  return (*state).pState.negative == (*state).pState.overflow;
}

static bool blt(SystemState *state) {
  return !bge(state);
}

static bool bgt(SystemState *state) {
  return bge(state) && !(beq(state));
}

static bool ble(SystemState *state) {
  return blt(state) || beq(state);
}

static bool bal(void) {
  return true;
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

static void logicalFlagUpdate(SystemState *state, int64_t result) {
  (*state).pState.negative = result < 0;
  (*state).pState.zero = result == 0;
  (*state).pState.carry = 0;
  (*state).pState.overflow = 0;
}

static void ands64_bics64(SystemState *state, uint64_t rd_reg, int64_t rn_dat,
                          int64_t rm_dat) {
  int64_t result = rn_dat & rm_dat;
  if (rd_reg != 31) write64bitReg(state, rd_reg, result);
  logicalFlagUpdate(state, result);
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
  if (rd_reg != 31) write32bitReg(state, rd_reg, result);
  logicalFlagUpdate(state, result);
}

static int generalPurposeRegisterNotFound(uint32_t rd) {
  fprintf(stderr, "General Purpose Register %"PRId32" does not exist!", rd);
  return 1;
}

static int executeImmediateDP(SystemState *state, const bool bits[]) {
  fprintf(stdout, "Immediate DP Instruction\n\n");
  uint32_t opi = getBitsSubsetUnsigned(bits, 25, 23);
  uint32_t opc = getBitsSubsetUnsigned(bits, 30, 29);
  bool sf = bits[31];
  uint32_t rd = getBitsSubsetUnsigned(bits, 4, 0);
  if (rd
      == 31); // if rd is 11111 in binary (rd thus codes for the zero register)
  else if (rd > GENERAL_PURPOSE_REGISTERS) {
    generalPurposeRegisterNotFound(rd);
  }
  switch (opi) {
    case 2://opi = 010 (ARITHMETIC)
    {
      bool sh = bits[22];
      uint32_t rn = getBitsSubsetUnsigned(bits, 9, 5);
      assert(rn < GENERAL_PURPOSE_REGISTERS);
      int32_t imm12 = (int32_t) getBitsSubsetUnsigned(bits, 21, 10);
      if (sh) {
        imm12 = imm12 << 12;
      }
      switch (opc) {
        case 0://opc = 00 (add)
          assert(rd < GENERAL_PURPOSE_REGISTERS);
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
            addsImmediateDP(64)
          } else {
            addsImmediateDP(32)
          }
          break;
        case 2://opc = 10 (sub)
          assert(rd < GENERAL_PURPOSE_REGISTERS);
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
            subsImmediateDP(64)
          } else {
            subsImmediateDP(32)
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
      uint16_t imm16 = getBitsSubsetUnsigned(bits, 20,
                                                    5); //assuming this number is meant to be signed

      int32_t shift = (int32_t) (hw * 16);
      uint64_t op64 = ((uint64_t) imm16) << shift;
      uint32_t op32 = ((uint32_t) imm16) << shift;

      switch (opc) {
        case 0://opc = 00 (movn)
          assert(rd < GENERAL_PURPOSE_REGISTERS);
          if (sf) //64 bit
          {
            (*state).generalPurpose[rd] = ~op64;
          } else {
            (*state).generalPurpose[rd] = ~op32;
          }

          break;
        case 2://opc = 10 (movz)
          assert(rd < GENERAL_PURPOSE_REGISTERS);
          if (sf) {
            (*state).generalPurpose[rd] = op64;
          } else {
            (*state).generalPurpose[rd] = op32;
          }

          break;
        case 3://opc = 11 (movk)
          assert(rd < GENERAL_PURPOSE_REGISTERS);
          if (sf) {//64 bit
            uint64_t val = (*state).generalPurpose[rd];

            (*state).generalPurpose[rd] = (val & ~(0xFFFFULL << shift))
                | ((uint64_t) (uint16_t) imm16 << shift);
          } else {
            uint32_t val = (uint32_t) (*state).generalPurpose[rd];

            (*state).generalPurpose[rd] =
                (val & ~(0xFFFF << shift))
                    | ((uint32_t) (uint16_t) imm16 << shift);
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
  fprintf(stdout, "Register DP Instruction\n\n");
  uint32_t m_opr = (bits[28] << 4) | getBitsSubsetUnsigned(bits, 24, 21);
  uint32_t opc = getBitsSubsetUnsigned(bits, 30, 29);
  uint32_t opc_n = (opc << 1) | bits[21];
  uint32_t opc_x = (opc << 1) | bits[15];
  bool sf = bits[31];

  uint32_t rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
  if (rd_reg
      == 31); // if rd is 11111 in binary (rd thus codes for the zero register)
  else if (rd_reg > GENERAL_PURPOSE_REGISTERS) {
    generalPurposeRegisterNotFound(rd_reg);
  }
  uint32_t shift = getBitsSubsetUnsigned(bits, 23, 22);
  uint32_t operand = getBitsSubsetUnsigned(bits, 15, 10);
  int64_t
      rn_dat = (int64_t) (*state).generalPurpose[getBitsSubsetUnsigned(bits,
                                                                       9,
                                                                       5)];
  int64_t
      rm_dat = (int64_t) (*state).generalPurpose[getBitsSubsetUnsigned(bits,
                                                                       20,
                                                                       16)];
  switch (m_opr) {
    case 8:
    case 10:
    case 12:
    case 14://M = 0, opr = 1xx0 (fall-through for unknowns)
      switch (opc) {
        case 0://opc = 00 (add)
          assert(rd_reg < GENERAL_PURPOSE_REGISTERS);
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
            if (rd_reg != 31) {
              (*state).generalPurpose[rd_reg] = res;
            }
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow64((int64_t) rn_dat,
                                                            (int64_t) rm_dat);
          } else {
            rm_dat = conditionalShiftForLogical32(shift, rm_dat, operand);
            int32_t res = (int32_t) rn_dat + (int32_t) rm_dat;
            if (rd_reg != 31) {
              (*state).generalPurpose[rd_reg] = zeroPad32BitSigned(res);
            }
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow32((int32_t) rn_dat,
                                                            (int32_t) rm_dat);
          }
          break;
        case 2://opc = 10 (sub)
          assert(rd_reg < GENERAL_PURPOSE_REGISTERS);
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
            int64_t minuend = rn_dat;
            int64_t subtrahend = rm_dat;

            int64_t res = minuend - subtrahend;
            if (rd_reg != 31) {
              (*state).generalPurpose[rd_reg] = res;
            }
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = minuend >= subtrahend;
            (*state).pState.overflow = checkOverUnderflow64((int64_t) rn_dat,
                                                            (int64_t) rm_dat);
          } else {

            rm_dat = conditionalShiftForLogical32(shift, rm_dat, operand);
            int32_t minuend = (int32_t) rn_dat;
            int32_t subtrahend = (int32_t) rm_dat;
            int32_t res = minuend - subtrahend;
            if (rd_reg != 31) {
              (*state).generalPurpose[rd_reg] = zeroPad32BitSigned(res);
            }
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = minuend >= subtrahend;
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
        rn_dat = (rn_reg == 31) ? 0 : read64bitReg(state, rn_reg);
        rm_dat = (rm_reg == 31) ? 0 : read64bitReg(state, rm_reg);
        shift = getBitsSubsetUnsigned(bits, 23, 22);
        printf("%016"PRIx64"\n", rm_dat);
        rm_dat =
            (int64_t) conditionalShiftForLogical64(shift, rm_dat, operand);
        printf("%016"PRIx64"\n", rm_dat);
        switch (opc_n) {
          case 0://opc = 00, N = 0 (and)
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
        uint32_t rn_reg = getBitsSubsetUnsigned(bits, 9, 5);
        uint32_t rm_reg = getBitsSubsetUnsigned(bits, 20, 16);
        int32_t rn_dat_32 = (rn_reg == 31) ? 0 : read32bitReg(state, rn_reg);
        int32_t rm_dat_32 = (rm_reg == 31) ? 0 : read32bitReg(state, rm_reg);
        shift = getBitsSubsetUnsigned(bits, 23, 22);
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
      rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
      uint32_t ra_reg = getBitsSubsetUnsigned(bits, 14, 10);
      if (bits[31]) {
        rn_dat = read64bitReg(state, getBitsSubsetUnsigned(bits, 9, 5));
        int64_t ra_dat = (ra_reg == 31) ? 0 : read64bitReg(state, ra_reg);
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
        rn_dat = read32bitReg(state, getBitsSubsetUnsigned(bits, 9, 5));
        int32_t ra_dat = (ra_reg == 31) ? 0 : read32bitReg(state, ra_reg);
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

  (*state).programCounter++;
  return 0;
}

static uint8_t readByteUnifiedMemory(SystemState *state,
                                     uint32_t address,
                                     int numberOfInstructions) {
  if (address < numberOfInstructions * 4) { //fetch from instruction memory
    uint32_t temp = (*state).instructionMemory[address / 4];
    switch (address % 4) {
      case 0:
        return temp & 0xff;
      case 1:
        return (temp & 0xff00) >> 8;
      case 2:
        return (temp & 0xff0000) >> (2 * 8);
      case 3:
        return (temp & 0xff000000) >> (3 * 8);
      default:
        return 0; // Due to how mod works- this will never happen
    }
  } else { //fetch from data memory
    return (*state).dataMemory[address - numberOfInstructions * 4];
  }
}

static void storeByteUnifiedMemory(SystemState *state,
                                   uint32_t address,
                                   int8_t value,
                                   int numberOfInstructions) {
  if (address < numberOfInstructions * 4) { //store to instruction memory
    switch (address % 4) {
      case 0:
        (*state).instructionMemory[address / 4] =
            (((*state).instructionMemory[address / 4]) & 0xFFFFFF00)
                ^ ((uint32_t) value);
        break;
      case 1:
        (*state).instructionMemory[address / 4] =
            (((*state).instructionMemory[address / 4]) & 0xFFFF00FF)
                ^ (((uint32_t) value) << 8);
        break;
      case 2:
        (*state).instructionMemory[address / 4] =
            (((*state).instructionMemory[address / 4]) & 0xFF00FFFF)
                ^ (((uint32_t) value) << (2 * 8));
        break;
      case 3:
        (*state).instructionMemory[address / 4] =
            (((*state).instructionMemory[address / 4]) & 0x00FFFFFF)
                ^ (((uint32_t) value) << (3 * 8));
        break;
      default:
        return; // Due to how mod works- this will never happen
    }
  } else { //store to data memory
    (*state).dataMemory[address - numberOfInstructions * 4] = value;
  }
}

static int executeSingleDataTransfer(SystemState *state,
                                     bool bits[],
                                     int numberOfInstructions) {
  fprintf(stdout, "Single Data Transfer Instruction\n\n");
  uint32_t rt = getBitsSubsetUnsigned(bits, 4, 0);
  if (bits[30]) {//64bit

    if (bits[22]) {//load

      uint64_t val = 0;
      uint32_t base = getMemAddress(state, bits);
      for (int i = 0; i < 8; i++) {
        val = val | (uint64_t) readByteUnifiedMemory(state,
                                                     base + i,
                                                     numberOfInstructions)
            << i * 8;
      }
      (*state).generalPurpose[rt] = val;

    } else {//store

      uint32_t base = getMemAddress(state, bits);
      uint64_t val = (*state).generalPurpose[rt];
      for (int i = 0; i < 8; i++) {
        uint64_t mask = (1 << ((8 * i + 7) - (8 * i) + 1)) -
            1;//mask of 1s with correct size
        mask = mask << (i * 8);  //shift mask to correct pos
        storeByteUnifiedMemory(state,
                               base + i,
                               (int8_t) ((val & mask) >> (i * 8)),
                               numberOfInstructions);
      }
      /*storeByteUnifiedMemory(state,
                             getMemAddress(bits),
                             (*state).generalPurpose[rt],
                             numberOfInstructions);*/
    }

  } else {//32bit

    if (bits[22]) {//load
      uint64_t val = 0;
      uint32_t base = getMemAddress(state, bits);
      for (int i = 0; i < 4; i++) {
        val = val | (uint32_t) readByteUnifiedMemory(state,
                                                     base + i,
                                                     numberOfInstructions)
            << i * 8;
      }
      (*state).generalPurpose[rt] = val;

    } else {//store

      uint32_t base = getMemAddress(state, bits);
      uint64_t val = (*state).generalPurpose[rt];
      for (int i = 0; i < 8; i++) {
        uint64_t mask = (1 << ((8 * i + 7) - (8 * i) + 1)) -
            1;//mask of 1s with correct size
        mask = mask << i * 8;  //shift mask to correct pos
        storeByteUnifiedMemory(state,
                               base + i,
                               (int8_t) ((val & mask) >> i * 8),
                               numberOfInstructions);
      }
      /*storeByteUnifiedMemory(state,
                             getMemAddress(bits),
                             (*state).generalPurpose[rt],
                             numberOfInstructions);*/
    }

  }


  (*state).programCounter++;
  return 0;
}

static int
executeLoadLiteral(SystemState *state,
                   bool bits[],
                   int numberOfInstructions) {
  fprintf(stdout, "Load Literal Instruction\n\n");
  uint32_t rt = getBitsSubsetUnsigned(bits, 4, 0);
  int32_t simm19 = getBitsSubsetSigned(bits, 23, 5);
  int32_t address = (int32_t) (4 * ((*state).programCounter + simm19));

  if (bits[30]) {//64bit
    uint64_t val = 0;
    int32_t base = address;
    for (int i = 0; i < 8; i++) {
      val = val | (uint64_t) readByteUnifiedMemory(state,
                                                   base + i,
                                                   numberOfInstructions)
          << i * 8;
    }
    (*state).generalPurpose[rt] = (uint64_t) val;
  } else {//32bit
    uint32_t val = 0;
    uint32_t base = getMemAddress(state, bits);
    for (int i = 0; i < 4; i++) {
      val = val | (uint32_t) readByteUnifiedMemory(state,
                                                   base + i,
                                                   numberOfInstructions)
          << i * 8;
    }
    (*state).generalPurpose[rt] = (uint32_t) val;
  }


  (*state).programCounter++;
  return 0;
}

static int executeBranch(SystemState *state, const bool bits[]) {
  fprintf(stdout, "Branch Instruction\n\n");
  uint32_t valForReg31to10 = getBitsSubsetUnsigned(bits, 31, 10);
  uint32_t valForReg4to0 = getBitsSubsetUnsigned(bits, 4, 0);
  uint32_t valForCond = getBitsSubsetUnsigned(bits, 31, 24);

  if (!bits[31] && !bits[30]) {//b
    b(state, bits);
  } else if (valForReg31to10 == 3508160 && valForReg4to0 == 0) {//br
    br(state, bits);
  } else if (valForCond == 84 && !bits[4]) {//b.cond
    uint32_t cond = getBitsSubsetUnsigned(bits, 3, 0);
    bool branchCondition;
    switch (cond) {
      case 0://cond = 0000 (beq)
        branchCondition = beq(state);
        break;
      case 1://cond = 0001 (bne)
        branchCondition = bne(state);
        break;
      case 10://cond = 1010 (bge)
        branchCondition = bge(state);
        break;
      case 11://cond = 1011 (blt)
        branchCondition = blt(state);
        break;
      case 12://cond = 1100 (bgt)
        branchCondition = bgt(state);
        break;
      case 13://cond = 1101 (ble)
        branchCondition = ble(state);
        break;
      case 14://cond = 1110 (bal)
        branchCondition = bal();
        break;
      default:
        return invalidInstruction();
    }
    conditionalBranch(state, bits, branchCondition);
  } else {
    return invalidInstruction();
  }


  return 0;
}

int execute(SystemState *state,
            bool bits[],
            int numberOfInstructions) { // Don't forget about `nop` !!
  if ((*state).instructionMemory[(*state).programCounter]
      == 0xD503201F) {// nop
    printf("Nop Instruction\n");
    (*state).programCounter++;
    return 0;
  } else if ((*state).instructionMemory[(*state).programCounter]
      == 0x8A000000) { // halt
    printf("Halt Instruction\n");
    return HALT;
  } else if (bits[28] && !bits[27] && !bits[26]) { // DP (Immediate)
    return executeImmediateDP(state, bits);
  } else if (bits[27] && !bits[26] && bits[25]) { // DP (Register)
    return executeRegisterDP(state, bits);
  } else if (bits[31] && bits[29] && bits[28] && bits[27] && !bits[26]
      && !bits[25] && !bits[23]) { // Single Data Transfer
    return executeSingleDataTransfer(state, bits, numberOfInstructions);
  } else if (!bits[31] && !bits[29] && bits[28] && bits[27] && !bits[26]
      && !bits[25] && !bits[24]) { // Load Literal
    return executeLoadLiteral(state, bits, numberOfInstructions);
  } else if (!bits[29] && bits[28] && !bits[27] && bits[26]) { // Branch
    return executeBranch(state, bits);
  }
  fprintf(stderr, "Invalid instruction type!");
  return 1;
}

void initialiseSystemState(SystemState *state, int numberOfInstructions,
                           const uint32_t instructions[]) {
  zeroArray((*state).generalPurpose, GENERAL_PURPOSE_REGISTERS)
  zeroArray((*state).instructionMemory, MAX_INSTRUCTIONS)
  for (int i = 0; i < numberOfInstructions; i++) {
    (*state).instructionMemory[i] = instructions[i];
  }
  zeroArray(&(*state).programCounter, 1)
  (*state).pState.negative = false;
  (*state).pState.zero = true;
  (*state).pState.carry = false;
  (*state).pState.overflow = false;
  zeroArray((*state).dataMemory, MEMORY_SIZE_BYTES)
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
  fprintf(file, "\nNon-Zero Memory:\n0x00000000");
  outputInstruction(file, (*state).instructionMemory[0]);
  for (int i = 1; i < numberOfInstructions; i++) {
    uint32_t val = (*state).instructionMemory[i];
    if (val != 0) {
      fprintf(file, "%#010"PRIx16, (int16_t) (i * 4));
      outputInstruction(file, val);
    }
  }
  for (int i = 0; i < MEMORY_SIZE_BYTES; i += 4) {
    int absoluteAddress = i + numberOfInstructions * 4;
    uint32_t val = 0;
    for (int j = 0; j < 4; j++) {
      val = val | (uint32_t) readByteUnifiedMemory(state,
                                                   absoluteAddress + j,
                                                   numberOfInstructions)
          << j * 8;
    }
    if (val != 0) {
      //fprintf(file, "Data Memory: ");
      fprintf(file, "%#010"PRIx16" : %08"
                    PRIx32"\n", (int16_t) absoluteAddress, val);
    }
  }
  fclose(file);
}
