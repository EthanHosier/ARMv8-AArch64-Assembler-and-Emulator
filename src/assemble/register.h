#ifndef REGISTER_H
#define REGISTER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int register_number;
  bool is_64_bit;
} Register;

typedef struct {
  bool is_register;
  union {
    Register reg;
    uint32_t imm;
  };
} Register_or_immediate;

typedef enum {
  Type_lsl,
  Type_lsr,
  Type_asr,
  Type_ror
} shift_type;

typedef struct {
  shift_type type;
  uint32_t amount;
} Shift;

#endif