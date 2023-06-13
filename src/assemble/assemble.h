#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <stdint.h>
#include "parser/parser.h"

typedef uint32_t (MACHINE_CODE_CONSTRUCTOR)(Parser_Tree);

MACHINE_CODE_CONSTRUCTOR register_dp_assembler;
MACHINE_CODE_CONSTRUCTOR immediate_dp_assembler;
MACHINE_CODE_CONSTRUCTOR single_data_transfer_assembler;
MACHINE_CODE_CONSTRUCTOR load_literal_assembler;
MACHINE_CODE_CONSTRUCTOR branch_assembler;


#endif

