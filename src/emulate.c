#include <stdio.h>
#include "utils.h"

int executeImmediateDP(SystemState *state, bool bits[]) {
  uint8_t opi = (bits[25] << 2) | (bits[24] << 1) | bits[23];
  uint8_t opc = (bits[30] << 1) | bits[29];
  switch (opi) {
    case 2:
      switch (opc) {
        case 0:
          //add
          break;
        case 1:
          //adds
          break;
        case 2:
          //sub
          break;
        case 3:
          //subs
          break;
        default:
          return invalidInstruction();
      }
      break;
    case 5:
      switch (opc) {
        case 0:
          //movn
          break;
        case 2:
          //movz
          break;
        case 3:
          //movk
          break;
        default:
          return invalidInstruction();
      }
      break;
    default:
      return invalidInstruction();
  }
  fprintf(stdout, "Immediate DP Instruction\n");
  (*state).programCounter++;
  return 0;
}

int executeRegisterDP(SystemState *state, bool bits[]) {
  /*
   * "and x0 x0 x0" is the halt instruction - use premature `return` statement
   * after overriding the value of the PC. This will avoid the PC being
   * incremented at the end of this function.
  */
  // Todo: Body
  uint8_t m_opr = (bits[28] << 4) | (bits[24] << 3) | (bits[23] << 2) | (bits[22] << 1) | bits[21];
  uint8_t opc = (bits[30] << 1) | bits[29];
  uint8_t opc_n = (opc << 1) | bits[21];
  uint8_t opc_x = (opc << 1) | bits[15];
  switch (m_opr) {
    case 8:
    case 10:
    case 12:
    case 14:
      switch (opc) {
        case 0:
          //add
          break;
        case 1:
          //adds
          break;
        case 2:
          //sub
          break;
        case 3:
          //subs
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
    case 7:
      switch (opc_n) {
        case 0:
          //and
          break;
        case 1:
          //bic
          break;
        case 2:
          //orr
          break;
        case 3:
          //orn
          break;
        case 4:
          //eor
          break;
        case 5:
          //eon
          break;
        case 6:
          //ands
          break;
        case 7:
          //bics
          break;
        default:
          return invalidInstruction();
      }
      break;
    case 24:
      switch (opc_x) {
        case 0:
          //madd
          break;
        case 1:
          //msub
          break;
        default:
          return invalidInstruction();
      }
      break;
    default:
      return invalidInstruction();
  }
  fprintf(stdout, "Register DP Instruction\n");
  (*state).programCounter++;
  return 0;
}

int executeSingleDataTransfer(SystemState *state, bool bits[]) {
  // Todo: Body
  fprintf(stdout, "Single Data Transfer Instruction\n");
  (*state).programCounter++;
  return 0;
}

int executeLoadLiteral(SystemState *state, bool bits[]) {
  // Todo: Body
  fprintf(stdout, "Load Literal Instruction\n");
  (*state).programCounter++;
  return 0;
}

int executeBranch(SystemState *state, bool bits[]) {
  // Todo: Body
  int valForRegLhs = 0;
  for (int i = 31; i >= 10; i--) {
    valForRegLhs = valForRegLhs << 1 | bits[i];
  }
  int valForRegRhs = 0;
  for (int i = 4; i >= 0; i--) {
    valForRegLhs = valForRegLhs << 1 | bits[i];
  }
  int valForCond = 0;
  for (int i = 31; i >= 24; i--) {
    valForCond = valForCond << 1 | bits[i];
  }


  if (!bits[31] && !bits[30]) {
    //b
  } else if (valForRegLhs == 3508160 && valForRegRhs == 0) {
    //br
  } else if (valForCond == 84 && !bits[4]) {
    //b.cond
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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments!\n");
    return 1;
  }
  uint32_t instructions[MAX_INSTRUCTIONS];
  int numberOfInstructions = 0;
  if (readBinaryFile(argv[1], instructions, &numberOfInstructions)) {
    return 1;
  }
  // printInstructions(instructions, numberOfInstructions);
  SystemState state;
  initialiseSystemState(&state);
  while (state.programCounter < numberOfInstructions) {
    bool bits[INSTRUCTION_SIZE_BITS];
    // Most significant bit has the highest index
    getBits(instructions[state.programCounter], bits);
    printInstruction(bits);
    if (execute(&state, bits)) {
      return 1;
    }
  }
  return 0;
}

uint64_t ror64(uint64_t operand, int bitsToRotate)
{
  assert(bitsToRotate <= 64);

  uint64_t ones = (UINT64_C(1) << bitsToRotate) - UINT64_C(1);
  uint64_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (64 - bitsToRotate));
  return operand;
}


uint32_t ror32(uint32_t operand, int bitsToRotate)
{
  assert(bitsToRotate <= 32);
  
  uint32_t ones = (UINT32_C(1) << bitsToRotate) - UINT32_C(1);
  uint32_t toAdd = ones & operand;
  operand = operand >> bitsToRotate;
  operand += (toAdd << (32 - bitsToRotate));
  return operand;
}