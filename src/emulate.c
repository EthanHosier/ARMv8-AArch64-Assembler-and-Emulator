#include <stdio.h>
#include "utils.h"

int executeImmediateDP(SystemState *state, const bool bits[]) {
  uint32_t opi = getBitsSubset(bits, 25, 23);
  uint32_t opc = getBitsSubset(bits, 30, 29);
  switch (opi) {
    case 2://opi = 010
      switch (opc) {
        case 0://opc = 00 (add)
          //add
          break;
        case 1://opc = 01 (adds)
          //adds
          break;
        case 2://opc = 10 (sub)
          //sub
          break;
        case 3://opc = 11 (subs)
          //subs
          break;
        default:
          return invalidInstruction();
      }
      break;
    case 5://opi = 101
      switch (opc) {
        case 0://opc = 00 (movn)
          //movn
          break;
        case 2://opc = 10 (movz)
          //movz
          break;
        case 3://opc = 11 (movk)
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

int executeRegisterDP(SystemState *state, const bool bits[]) {
  /*
   * "and x0 x0 x0" is the halt instruction - use premature `return` statement
   * after overriding the value of the PC. This will avoid the PC being
   * incremented at the end of this function.
  */
  // Todo: Body
  uint32_t m_opr = (bits[28] << 4) | getBitsSubset(bits, 24, 21);
  uint32_t opc = getBitsSubset(bits, 30, 29);;
  uint32_t opc_n = (opc << 1) | bits[21];
  uint32_t opc_x = (opc << 1) | bits[15];
  switch (m_opr) {
    case 8:
    case 10:
    case 12:
    case 14://M = 0, opr = 1xx0 (fall-through for unknowns)
      switch (opc) {
        case 0://opc = 00 (add)
          //add
          break;
        case 1://opc = 01 (adds)
          //adds
          break;
        case 2://opc = 10 (sub)
          //sub
          break;
        case 3://opc  = 11 (subs)
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
    case 7://M = 0, opr = 0xxx (fall-through for unknowns)
      switch (opc_n) {
        case 0://opc = 00, N = 0 (and)
          //and
          break;
        case 1://opc = 00, N = 1 (bic)
          //bic
          break;
        case 2://opc = 01, N = 0 (orr)
          //orr
          break;
        case 3://opc = 01, N = 1 (orn)
          //orn
          break;
        case 4://opc = 10, N = 0 (eor)
          //eor
          break;
        case 5://opc = 10, N = 1 (eon)
          //eon
          break;
        case 6://opc = 11, N = 0 (ands)
          //ands
          break;
        case 7://opc = 11, N = 1 (bics)
          //bics
          break;
        default:
          return invalidInstruction();
      }
      break;
    case 24://M = 1, opr = 1000
      switch (opc_x) {
        case 0://opc = 00, x = 0 (madd)
          //madd
          break;
        case 1://opc = 00, x = 1 (msub)
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
  uint32_t rt = getBitsSubset(bits, 4, 0);
  if (bits[22]) {//load
    (*state).generalPurpose[rt] = (*state).primaryMemory[getMemAddress(bits)];
  } else {//store
    (*state).primaryMemory[getMemAddress(bits)] = (*state).generalPurpose[rt];
  }

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

int executeBranch(SystemState *state, const bool bits[]) {
  // Todo: Body
  uint32_t valForReg31to10 = getBitsSubset(bits, 31, 10);
  uint32_t valForReg4to0 = getBitsSubset(bits, 4, 0);
  uint32_t valForCond = getBitsSubset(bits, 31, 24);

  if (!bits[31] && !bits[30]) {//b
    b(state, bits);
  } else if (valForReg31to10 == 3508160 && valForReg4to0 == 0) {//br
    br(state, bits);
  } else if (valForCond == 84 && !bits[4]) {//b.cond
    uint32_t cond = getBitsSubset(bits, 3, 0);
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
    return 0;
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

