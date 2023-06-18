#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "../parser/parser.h"

typedef uint32_t *(*decode_function)(Parser_Tree *tree);

extern uint32_t *decoder(Parser_Tree *);

#endif