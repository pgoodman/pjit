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
struct StructureFieldInfo;

namespace mir {

class Symbol;


// Medium-level IR instruction operation codes.
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
  OP_LOAD_FIELD,
  OP_STORE_FIELD,
  OP_CONVERT_TYPE,
  OP_ASSIGN,

  OP_CCALL1,
  OP_CCALL2,
  OP_CCALL3,

  // Tail-call to the next instruction decode cycle. The values of the
  // variables marked as persistent will be guaranteed to be available to
  // the next iteration of instruction fetch / decode / execute.
  OP_NEXT
};


union Operand {
  const Symbol *symbol;
  const StructureFieldInfo *field;
};


// A 2- or 3-operand instruction for the medium-level IR.
class Instruction {
 public:
  enum {
    kMaxNumOperands = 3
  };

  const Operation operation;
  Instruction *prev;
  Instruction *next;

  Operand operands[kMaxNumOperands];

  inline Instruction(Operation op, std::initializer_list<const void *> ops)
      : operation(op),
        prev(nullptr),
        next(nullptr) {
    memcpy(&(operands[0]), ops.begin(), ops.size() * sizeof(const void *));
  }

  Instruction(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(Instruction);
};


}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_INSTRUCTION_H_
