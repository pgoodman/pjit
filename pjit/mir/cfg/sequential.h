/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * sequential.h
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CFG_SEQUENTIAL_H_
#define PJIT_MIR_CFG_SEQUENTIAL_H_

#include "pjit/base/base.h"
#include "pjit/mir/cfg/basic-block.h"
#include "pjit/mir/cfg/control-flow-graph.h"

namespace pjit {

namespace hir {
class IfStatementBuilder;
}

namespace mir {

class MultiWayBranchArm;
class MultiWayBranchControlFlowGraph;
class MultiWayFirstBasicBlockFinder;
class MultiWayPredecessorBasicBlockFinder;


// Represents a straight-line sequence of code (a basic block), followed by
// another control-flow graph.
class SequentialControlFlowGraph : public ControlFlowGraph {
 public:
  SequentialControlFlowGraph(Context *context, ControlFlowGraph *parent);
  virtual ~SequentialControlFlowGraph(void) = default;

  inline void Append(Instruction *in) {
    bb.Append(in);
  }

  inline void Prepend(Instruction *in) {
    bb.Prepend(in);
  }

  virtual void VisitPreOrder(ControlFlowGraphVisitor *);
  virtual void VisitPostOrder(ControlFlowGraphVisitor *);

 protected:
  virtual void VisitFirst(BasicBlockVisitor *);
  virtual void VisitPredecessor(ControlFlowGraph *, BasicBlockVisitor *);

 private:
  friend class hir::IfStatementBuilder;
  friend class Context;
  friend class ConditionalControlFlowGraph;
  friend class MultiWayBranchArm;
  friend class MultiWayBranchControlFlowGraph;
  friend class MultiWayFirstBasicBlockFinder;
  friend class MultiWayPredecessorBasicBlockFinder;

  BasicBlock bb;
  ControlFlowGraph *successor;

  SequentialControlFlowGraph(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(SequentialControlFlowGraph);
};

}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_CFG_SEQUENTIAL_H_
