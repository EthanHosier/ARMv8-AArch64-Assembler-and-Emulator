#include <stdio.h>
#include "utils.h"
#include <assert.h>

int executeImmediateDP(SystemState *state, const bool bits[]) {
  uint32_t opi = getBitsSubsetUnsigned(bits, 25, 23);
  uint32_t opc = getBitsSubsetUnsigned(bits, 30, 29);
  bool sf = bits[31];
  uint32_t rd = getBitsSubsetUnsigned(bits, 4, 0);
  switch (opi) {
    case 2://opi = 010 (ARITHMETIC)
      assert(rd < 32);
      bool sh = bits[22];
      uint32_t rn = getBitsSubsetUnsigned(bits, 9, 5);
      assert(rn < 32);
      int32_t imm12 =  getBitsSubsetSigned(bits, 21, 10);
      if (sh) {
        imm12 = imm12 << 12;
      }
      switch (opc) {
        case 0://opc = 00 (add)
          if (sf) {
            (*state).generalPurpose[rd] = ((int64_t) (*state).generalPurpose[rn]) + imm12;
          } else {
            (*state).generalPurpose[rd] = zeroPad32BitSigned(((int32_t) (*state).generalPurpose[rn]) + imm12);
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
            (*state).pState.overflow = checkOverUnderflow64((int64_t) (*state).generalPurpose[rn],(int64_t) imm12);
          } else {
            int32_t res = (int32_t) ((*state).generalPurpose[rn]) + imm12;
            (*state).generalPurpose[rd] = zeroPad32BitSigned(res);
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow32((int32_t) ((*state).generalPurpose[rn]), imm12);
          }
          break;
        case 2://opc = 10 (sub)
          if (sf) {
            (*state).generalPurpose[rd] = ((int64_t) (*state).generalPurpose[rn]) - imm12;
          } else {
            (*state).generalPurpose[rd] = zeroPad32BitSigned(((int32_t) (*state).generalPurpose[rn]) - imm12);
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
            (*state).pState.overflow = checkOverUnderflow64((int64_t) (*state).generalPurpose[rn],(int64_t) imm12);
          } else {
            int32_t res = (int32_t) ((*state).generalPurpose[rn]) - imm12;
            (*state).generalPurpose[rd] = zeroPad32BitSigned(res);
            (*state).pState.negative = res < 0;
            (*state).pState.zero = res == 0;
            // Come back to this later
            (*state).pState.carry = 0;
            (*state).pState.overflow = checkOverUnderflow32((int32_t) ((*state).generalPurpose[rn]), imm12);
          }
          break;
        default:
          return invalidInstruction();
      }
      break;
    case 5://opi = 101
      uint32_t hw = getBitsSubsetUnsigned(bits,22,21);
      int16_t imm16 = getBitsSubsetSigned(bits, 20, 5); //assuming this number is meant to be signed
      
      int32_t shift = (hw*16);
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
          if(sf){
            (*state).generalPurpose[rd] = op64;            
          } else {
            (*state).generalPurpose[rd] = op32;
          }

          break;
        case 3://opc = 11 (movk)
          //TODO: add when 11111 case (for entire instruction set)
          //TESTTTTTTTTTTTTTTTTT
          if(sf){//64 bit
            uint64_t val = (*state).generalPurpose[rd];
            uint64_t top = val / (1 << (shift + 15)); //might be +14 idk
            uint64_t bottom = val % (1 << (shift - 1)); //i think -1

            uint64_t joined = top | (imm16 << shift) | bottom;   
            (*state).generalPurpose[rd] = joined;
          } else {
            uint32_t val = (uint32_t) (*state).generalPurpose[rd];
            uint32_t top = val / (1 << (shift + 15)); //might be +14 idk
            uint32_t bottom = val % (1 << (shift - 1)); //i think -1

            uint64_t joined = (uint64_t) (top | (imm16 << shift) | bottom);   
            (*state).generalPurpose[rd] = joined;
          }
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
  uint32_t m_opr = (bits[28] << 4) | getBitsSubsetUnsigned(bits, 24, 21);
  uint32_t opc = getBitsSubsetUnsigned(bits, 30, 29);;
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
      bool sf = bits[31];
      if (sf) {
        uint32_t rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        uint64_t rn_dat = read64bitreg(state, getBitsSubsetUnsigned(bits, 9, 5));
        uint64_t rm_dat = read64bitreg(state, getBitsSubsetUnsigned(bits, 20, 16));
        uint32_t shift = getBitsSubsetUnsigned(bits, 23, 22);
        int32_t operand = getBitsSubsetSigned(bits, 15, 10);
        rm_dat = conditionalShiftForLogical32(shift, rm_dat, operand);
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
        uint32_t rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        uint32_t rn_dat = read32bitreg(state, getBitsSubsetUnsigned(bits, 9, 5));
        uint32_t rm_dat = read32bitreg(state, getBitsSubsetUnsigned(bits, 20, 16));
        uint32_t shift = getBitsSubsetUnsigned(bits, 23, 22);
        int32_t operand = getBitsSubsetSigned(bits, 15, 10);
        rm_dat = conditionalShiftForLogical32(shift, rm_dat, operand);
        switch (opc_n) {
          case 0://opc = 00, N = 0 (and)
            and32_bic32(state, rd_reg, rn_dat, rm_dat);
            break;
          case 1://opc = 00, N = 1 (bic)
            //bic
            and32_bic32(state, rd_reg, rn_dat, ~rm_dat);
            break;
          case 2://opc = 01, N = 0 (orr)
            //orr
            orr32_orn32(state, rd_reg, rn_dat, rm_dat);
            break;
          case 3://opc = 01, N = 1 (orn)
            //orn
            orr32_orn32(state, rd_reg, rn_dat, ~rm_dat);
            break;
          case 4://opc = 10, N = 0 (eor)
            //eor
            eor32_eon32(state, rd_reg, rn_dat, rm_dat);
            break;
          case 5://opc = 10, N = 1 (eon)
            //eon
            eor32_eon32(state, rd_reg, rn_dat, ~rm_dat);
            break;
          case 6://opc = 11, N = 0 (ands)
            //ands
            ands32_bics32(state, rd_reg, rn_dat, rm_dat);
            break;
          case 7://opc = 11, N = 1 (bics)
            //bics
            ands32_bics32(state, rd_reg, rn_dat, ~rm_dat);
            break;
          default:
            return invalidInstruction();
        }
      }
      break;
    case 24://M = 1, opr = 1000
      if (bits[31]) {
        uint64_t rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        int64_t rn_dat = read64bitreg(state, getBitsSubsetUnsigned(bits, 9, 5));
        int64_t ra_dat = read64bitreg(state, getBitsSubsetUnsigned(bits, 14, 10));
        int64_t rm_dat = read64bitreg(state, getBitsSubsetUnsigned(bits, 20, 16));
        switch (opc_x) {
          case 0://opc = 00, x = 0 (madd)
            //madd
            write64bitreg(state, rd_reg, ra_dat + (rn_dat * rm_dat));
            break;
          case 1://opc = 00, x = 1 (msub)
            //msub
            write64bitreg(state, rd_reg, ra_dat - (rn_dat * rm_dat));
            break;
          default:
            return invalidInstruction();
        }
      } else {
        uint32_t rd_reg = getBitsSubsetUnsigned(bits, 4, 0);
        int32_t rn_dat = read32bitreg(state, getBitsSubsetUnsigned(bits, 9, 5));
        int32_t ra_dat = read32bitreg(state, getBitsSubsetUnsigned(bits, 14, 10));
        int32_t rm_dat = read32bitreg(state, getBitsSubsetUnsigned(bits, 20, 16));
        switch (opc_x) {
          case 0://opc = 00, x = 0 (madd)
            //madd
            write32bitreg(state, rd_reg, ra_dat + (rn_dat * rm_dat));
            break;
          case 1://opc = 00, x = 1 (msub)
            //msub
            write32bitreg(state, rd_reg, ra_dat - (rn_dat * rm_dat));
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

int executeSingleDataTransfer(SystemState *state, bool bits[]) {
  uint32_t rt = getBitsSubsetUnsigned(bits, 4, 0);
  if (bits[30]) {//64bit

    if (bits[22]) {//load

      uint64_t val = 0;
      int base = getMemAddress(bits);
      for (int i = 0; i < 8; i ++) {
        val = val | (*state).primaryMemory[base + i] << i*8;
      }
      (*state).generalPurpose[rt] = val;

    } else {//store

      int base = getMemAddress(bits);
      uint64_t val = (*state).generalPurpose[rt];
      for (int i = 0; i < 8; i ++) {
        unsigned int mask = (1 << ((8*i + 7) - (8*i) + 1)) - 1;//mask of 1s with correct size
        mask = mask << i;  //shift mask to correct pos
        (*state).primaryMemory[base + i] = (val & mask) >> i;
      }
      (*state).primaryMemory[getMemAddress(bits)] = (*state).generalPurpose[rt];
    }

  } else {//32bit
    
    if (bits[22]) {//load
      uint64_t val = 0;
      int base = getMemAddress(bits);
      for (int i = 0; i < 4; i ++) {
        val = val | (*state).primaryMemory[base + i] << i*8;
      }
      (*state).generalPurpose[rt] = val;
      
    } else {//store

      int base = getMemAddress(bits);
      uint64_t val = (*state).generalPurpose[rt];
      for (int i = 0; i < 8; i ++) {
        unsigned int mask = (1 << ((8*i + 7) - (8*i) + 1)) - 1;//mask of 1s with correct size
        mask = mask << i;  //shift mask to correct pos
        (*state).primaryMemory[base + i] = (val & mask) >> i;
      }
      (*state).primaryMemory[getMemAddress(bits)] = (*state).generalPurpose[rt];
    }

  }
  

  fprintf(stdout, "Single Data Transfer Instruction\n");
  (*state).programCounter++;
  return 0;
}

int executeLoadLiteral(SystemState *state, bool bits[]) {
  // Todo: Body
  uint32_t rt = getBitsSubsetUnsigned(bits, 4, 0);
  int32_t simm19 = getBitsSubsetSigned(bits, 23, 5);
  int32_t address = (*state).programCounter + simm19;

  if (bits[30]) {//64bit

    uint64_t val = 0;
    int base = address;
    for (int i = 0; i < 8; i ++) {
      val = val | (*state).primaryMemory[base + i] << i*8;
    }
    (*state).generalPurpose[rt] = val;

  } else {//32bit
    
    uint64_t val = 0;
    int base = getMemAddress(bits);
    for (int i = 0; i < 4; i ++) {
      val = val | (*state).primaryMemory[base + i] << i*8;
    }
    (*state).generalPurpose[rt] = val;

  }

    fprintf(stdout, "Load Literal Instruction\n");
    (*state).programCounter++;
    return 0;
}

int executeBranch(SystemState *state, const bool bits[]) {
  // Todo: Body
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
  outputToFile(&state);
  return 0;
}

