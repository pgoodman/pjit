/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * main.cc
 *
 *  Created on: 2013-12-19
 *      Author: Peter Goodman
 */


#include "pjit/hir/hir-to-mir.h"
#include "pjit/mir/logging.h"


static pjit::mir::Context C;


int main(int argc_, const char **argv_) {
  using namespace pjit::hir;

  PJIT_HIR_DECLARE(C, (int), argc);
  PJIT_HIR_DECLARE(C, (const char **), argv);

  ASSIGN(C, argc, argc_);
  ASSIGN(C, argv, argv_);

  PJIT_HIR_IF(C, COMPARE_GT(C, argc, 0))
    PJIT_HIR_DECLARE(C, (const char *), arg);
    ASSIGN(C, arg, LOAD_MEMORY(C, argv));
    ASSIGN(C, argc, ADD(C, argc, 1));

  PJIT_HIR_ELSE(C)
    PJIT_HIR_SWITCH(C, argc)
      PJIT_HIR_CASE(C, 1)

      PJIT_HIR_END_CASE
      PJIT_HIR_CASE(C, 2)

      PJIT_HIR_END_CASE
      PJIT_HIR_CASE(C, 3)

      PJIT_HIR_END_CASE
      PJIT_HIR_CASE(C, 4)

      PJIT_HIR_END_CASE
    PJIT_HIR_END_SWITCH
  PJIT_HIR_END_IF

  pjit::Log(pjit::LogLevel::LogOutput, &C);

  return 0;
}

