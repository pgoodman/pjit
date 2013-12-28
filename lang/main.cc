/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * main.cc
 *
 *  Created on: 2013-12-19
 *      Author: Peter Goodman
 */


#include "pjit/hir/hir-to-mir.h"
#include "pjit/mir/logging.h"

using namespace pjit;


static mir::Context C;


int main(int argc_, const char **argv_) {
  PJIT_HIR_DECLARE(C, (int), argc);
  PJIT_HIR_DECLARE(C, (const char **), argv);

  Assign(C, argc, argc_);
  Assign(C, argv, argv_);

  PJIT_HIR_IF(C, CompareGreaterThan(C, argc, 0))
    PJIT_HIR_DECLARE(C, (const char *), arg);
    Assign(C, arg, LoadMemory(C, argv));
    Assign(C, argc, Add(C, argc, 1));
  PJIT_HIR_ELSE(C)
    Assign(C, argv, nullptr);
  PJIT_HIR_END_IF

  Log(LogLevel::LogOutput, &C);

  return 0;
}

