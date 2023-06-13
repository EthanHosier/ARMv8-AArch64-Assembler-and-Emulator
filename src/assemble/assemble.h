#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdint.h>
#include "parser/parser.h"

typedef uint32_t (MACHINE_CODE_CONSTRUCTOR)(Parser_Tree);

extern MACHINE_CODE_CONSTRUCTOR register_dp_assembler;
extern MACHINE_CODE_CONSTRUCTOR immediate_dp_assembler;
extern MACHINE_CODE_CONSTRUCTOR single_data_transfer_assembler;
extern MACHINE_CODE_CONSTRUCTOR load_literal_assembler;
extern MACHINE_CODE_CONSTRUCTOR branch_assembler;

#endif
