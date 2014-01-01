/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * basic-register-machine.cc
 *
 *  Created on: 2013-12-19
 *      Author: Peter Goodman
 */

#include <cstdio>


#include "pjit/base/unsafe-cast.h"
#include "pjit/hir/hir-to-mir.h"
#include "pjit/mir/logging.h"


static pjit::mir::Context C;


// Simple register class for implementing register windowing.
enum REG : int {
  I1 = 0,
  I2 = 1,
  I3 = 2,
  I4 = 3,
  R1 = 4,
  R2 = 5,
  R3 = 6,
  R4 = 7,
  O1 = 8,  // shadows I1
  O2 = 9,  // shadows I2
  O3 = 10,  // shadows I3
  O4 = 11,  // shadows I4
  NUM_REGS = O1
};


// Simple operand within a 3-operand instruction format.
union OP {
  REG reg;
  int disp;
  int value;

  OP(REG register_id)
      : reg(register_id) {}

  OP(int displacement)
      : disp(displacement) {}

  OP(void)
      : disp(0) {}
};


// Opcodes.
enum OPC {
  ASSIGN_VAL,
  ASSIGN_REG,
  ADD,
  INC,
  JUMP_IF_ZERO,
  CALL,
  RET,
};


// Simple 3-operand RISC-like instruction.
struct INS {
  OPC opcode;
  OP operands[3];
};


// Machine state at a point in time.
struct MSTATE {
  int regs[REG::NUM_REGS];
};


namespace pjit {

PJIT_DECLARE_ENUM_TYPE_INFO(REG);
PJIT_DEFINE_ENUM_TYPE_INFO(REG);

PJIT_DECLARE_ENUM_TYPE_INFO(OPC);
PJIT_DEFINE_ENUM_TYPE_INFO(OPC);

PJIT_DECLARE_UNION_TYPE_INFO(OP);
PJIT_DEFINE_UNION_TYPE_INFO(OP,
    PJIT_DEFINE_FIELD((REG), reg),
    PJIT_DEFINE_FIELD((int), disp),
    PJIT_DEFINE_FIELD((int), value));

PJIT_DECLARE_STRUCTURE_TYPE_INFO(INS);
PJIT_DEFINE_STRUCTURE_TYPE_INFO(INS,
    PJIT_DEFINE_FIELD((OPC), opcode),
    PJIT_DEFINE_ARRAY_FIELD((OP), 3, operands));

PJIT_DECLARE_STRUCTURE_TYPE_INFO(MSTATE);
PJIT_DEFINE_STRUCTURE_TYPE_INFO(MSTATE,
    PJIT_DEFINE_ARRAY_FIELD((REG), REG::NUM_REGS, regs));

}  // namespace pjit


static int REGISTER_FILE[sizeof(MSTATE) * 30] = {0};


static int fib(int i) {
  if (!i) {
    return 1;
  } else if (1 == i) {
    return 1;
  } else {
    return fib(i - 1) + fib(i - 2);
  }
}


static INS FIBONNACI[] = {
  {OPC::JUMP_IF_ZERO, {{REG::I1}, {9}}},
  {OPC::INC, {{REG::I1}, {-1}}},
  {OPC::JUMP_IF_ZERO, {{REG::I1}, {7}}},
  {OPC::ASSIGN_REG, {{REG::O1}, {REG::I1}}},
  {OPC::CALL, {{-5}, {REG::R1}}},
  {OPC::INC, {{REG::I1}, {-1}}},
  {OPC::ASSIGN_REG, {{REG::O1}, {REG::I1}}},
  {OPC::CALL, {{-8}, {REG::R2}}},
  {OPC::ADD, {{REG::R1}, {REG::R1}, {REG::R2}}},
  {OPC::RET, {{REG::R1}}},
  {OPC::ASSIGN_VAL, {{REG::R1}, {1}}},
  {OPC::RET, {{REG::R1}}}
};


static int eval(INS *ins, MSTATE *state) {
  for (;;) {
    const INS in(*ins++);
    switch (in.opcode) {
      case OPC::ASSIGN_VAL:
        state->regs[in.operands[0].reg] = in.operands[1].value;
        break;
      case OPC::ASSIGN_REG:
        state->regs[in.operands[0].reg] = state->regs[in.operands[1].reg];
        break;
      case OPC::ADD:
        state->regs[in.operands[0].reg] = state->regs[in.operands[1].reg] +
                                          state->regs[in.operands[2].reg];
        break;
      case OPC::INC:
        state->regs[in.operands[0].reg] += in.operands[1].value;
        break;
      case OPC::JUMP_IF_ZERO:
        if (0 == state->regs[in.operands[0].reg]) {
          ins = &(ins[in.operands[1].disp]);
        }
        break;
      case OPC::CALL:
        state->regs[in.operands[1].reg] = eval(
            &(ins[in.operands[0].disp]), state + 1);
        break;
      case OPC::RET:
        return state->regs[in.operands[0].reg];
    }
  }
  return 0;
}





static void pjit_eval_ins(void) {
  using namespace pjit::hir;

  PJIT_HIR_DECLARE(C, (INS *), ins);
  PJIT_HIR_DECLARE(C, (INS), in);

  ASSIGN(C, in, LOAD_MEMORY(C, ins));
  ASSIGN(C, ins, pjit::hir::ADD(C, ins, 1));

  PJIT_HIR_SWITCH(C, PJIT_HIR_ACCESS_FIELD(C, ins, opcode))
    PJIT_HIR_CASE(C, OPC::ASSIGN_VAL)
    PJIT_HIR_END_CASE

    PJIT_HIR_CASE(C, OPC::ASSIGN_REG)
    PJIT_HIR_END_CASE

    PJIT_HIR_CASE(C, OPC::ADD)
    PJIT_HIR_END_CASE

    PJIT_HIR_CASE(C, OPC::INC)
    PJIT_HIR_END_CASE

    PJIT_HIR_CASE(C, OPC::JUMP_IF_ZERO)
    PJIT_HIR_END_CASE

    PJIT_HIR_CASE(C, OPC::CALL)
    PJIT_HIR_END_CASE

    PJIT_HIR_CASE(C, OPC::RET)
    PJIT_HIR_END_CASE
  PJIT_HIR_END_SWITCH

#if 0
      switch (in.opcode) {
        case OPC::ASSIGN_VAL:
          state->regs[in.operands[0].reg] = in.operands[1].value;
          break;
        case OPC::ASSIGN_REG:
          state->regs[in.operands[0].reg] = state->regs[in.operands[1].reg];
          break;
        case OPC::ADD:
          state->regs[in.operands[0].reg] = state->regs[in.operands[1].reg] +
                                            state->regs[in.operands[2].reg];
          break;
        case OPC::INC:
          state->regs[in.operands[0].reg] += in.operands[1].value;
          break;
        case OPC::JUMP_IF_ZERO:
          if (0 == state->regs[in.operands[0].reg]) {
            ins = &(ins[in.operands[1].disp]);
          }
          break;
        case OPC::CALL:
          state->regs[in.operands[1].reg] = eval(
              &(ins[in.operands[0].disp]), state + 1);
          break;
        case OPC::RET:
          return state->regs[in.operands[0].reg];
      }
#endif
}


int main(void) {

  MSTATE *frame = pjit::UnsafeCast<MSTATE *>(&(REGISTER_FILE[0]));

  printf("/*\n");
  for (int i(0); i < 10; ++i) {
    printf("fib(%d) = %d\n", i, fib(i));
    frame->regs[REG::I1] = i;
    printf("eval-fib(%d) = %d\n\n", i, eval(&(FIBONNACI[0]), frame));
  }
  printf("*/\n");

  pjit_eval_ins();
  C.GarbageCollect();
  pjit::Log(pjit::LogLevel::LogWarning, &C);

  return 0;
}

