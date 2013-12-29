/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * instruction.h
 *
 *  Created on: 2013-12-22
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_INSTRUCTION_H_
#define PJIT_MIR_INSTRUCTION_H_

#include <initializer_list>

#include "pjit/base/base.h"
#include "pjit/base/libc.h"

namespace pjit {

struct TypeInfo;
class Symbol;

namespace mir {

enum class Operation {
#define PJIT_DECLARE_BINARY_OPERATOR(opcode, _) \
  PJIT_CAT(OP_, opcode),
#define PJIT_DECLARE_UNARY_OPERATOR(opcode, _) \
  PJIT_CAT(OP_, opcode),
#include "pjit/mir/operator.h"
#undef PJIT_DECLARE_BINARY_OPERATOR
#undef PJIT_DECLARE_UNARY_OPERATOR
  OP_LOAD_MEMORY,
  OP_STORE_MEMORY,
  OP_CONVERT_TYPE,
  OP_ASSIGN,
  OP_NOP
};


// A 2- or 3-operand instruction for the medium-level IR.
class Instruction {
 public:

  const Operation operation;
  Instruction *prev;
  Instruction *next;

  const Symbol *operands[3];

  inline Instruction(Operation op, std::initializer_list<const Symbol *> ops)
      : operation(op),
        prev(nullptr),
        next(nullptr) {
    memcpy(&(operands[0]), ops.begin(), ops.size() * sizeof(const Symbol *));
  }

  Instruction(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(Instruction);
};


}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_INSTRUCTION_H_
