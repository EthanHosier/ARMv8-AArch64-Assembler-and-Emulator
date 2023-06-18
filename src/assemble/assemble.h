#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdint.h>
#include "parser/parser.h"

typedef uint32_t (machine_code_constructor)(Parser_Tree);

//extern machine_code_constructor register_dp_assembler;
//extern machine_code_constructor immediate_dp_assembler;
//extern machine_code_constructor single_data_transfer_assembler;
//extern machine_code_constructor load_literal_assembler;
//extern machine_code_constructor branch_assembler;

uint32_t *buildAddAddsSubSubsImm(Parser_Tree *tree);
uint32_t *buildMovkMovnMovz(Parser_Tree *tree);

uint32_t *buildBinaryDPImm(uint32_t sf,
                          uint32_t opc,
                          uint32_t opi,
                          uint32_t operand,
                          uint32_t rd);

uint32_t *buildAddAddsSubSubsReg(Parser_Tree *tree);
uint32_t *buildAndAndsBicBicsEorOrrEonOrn(Parser_Tree *tree);
uint32_t *buildMaddMsub(Parser_Tree *tree);

uint32_t *buildBinaryDPReg(uint32_t sf,
                          uint32_t opc,
                          uint32_t m,
                          uint32_t opr,
                          uint32_t rm,
                          uint32_t operand,
                          uint32_t rn,
                          uint32_t rd);

uint32_t *buildBinarySDT(uint32_t sf,
                        uint32_t u,
                        uint32_t l,
                        uint32_t offset,
                        uint32_t xn,
                        uint32_t rt);

uint32_t *buildBinaryLoadLiteral(uint32_t sf,
                                uint32_t simm19,
                                uint32_t rt);

uint32_t *buildBinaryBranchUnconditional(uint32_t simm26);
uint32_t *buildBinaryBranchRegister(uint32_t xn);
uint32_t *buildBinaryBranchConditional(uint32_t simm19, uint32_t cond);

static Register *createZeroRegister(bool is64Bit);

#endif
