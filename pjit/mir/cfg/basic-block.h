/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * basic-block.h
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CFG_BASIC_BLOCK_H_
#define PJIT_MIR_CFG_BASIC_BLOCK_H_

#include "pjit/base/base.h"

namespace pjit {
namespace mir {

class Instruction;
class SequentialControlFlowGraph;


// Represents a short, straight-line sequence of instructions. Basic blocks do
// not contain control-flow instructions (except function calls), and all
// control flow is defined in terms of the in-memory structure of the control-
// flow graphs containing basic blocks.
class BasicBlock {
 public:
  void Append(Instruction *);
  void Prepend(Instruction *);

  Instruction *first;
  Instruction *last;

 private:
  friend class SequentialControlFlowGraph;

  BasicBlock(void);

  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};


// Represents a generic visitor for operating on an individual basic block.
// Unlike control-flow graph visitors, basic block visitors don't have a
// concept of pre- or post-order, and it isn't meaningful. Basic block visitors
// are primarily used to inspect the successors and predecessors of a given
// basic block, *while* that basic block is being inspected from within a
// control-flow graph visitor.
class BasicBlockVisitor {
 public:
  BasicBlockVisitor(void) = default;
  virtual ~BasicBlockVisitor(void) = default;
  virtual void Visit(BasicBlock *bb) = 0;

 private:
  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlockVisitor);
};


}  // namespace mir
}  // namespace pjit

#endif  // PJIT_MIR_CFG_BASIC_BLOCK_H_
