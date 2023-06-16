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

#endif
