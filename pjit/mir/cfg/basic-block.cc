/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * basic-block.cc
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#include "pjit/mir/instruction.h"
#include "pjit/mir/cfg/basic-block.h"

namespace pjit {
namespace mir {

BasicBlock::BasicBlock(void)
    : first(nullptr),
      last(nullptr) {}


void BasicBlock::Append(Instruction *in) {
  in->prev = last;
  in->next = nullptr;

  if (last) {
    last->next = in;
  } else {
    first = in;
  }

  last = in;
}


void BasicBlock::Prepend(Instruction *in) {
  in->prev = nullptr;
  in->next = first;

  if (first) {
    first->prev = in;
  } else {
    last = in;
  }

  first = in;
}

}  // namespace mir
}  // namespace pjit

