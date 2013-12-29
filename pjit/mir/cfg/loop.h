/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * loop.h
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CFG_LOOP_H_
#define PJIT_MIR_CFG_LOOP_H_

#include "pjit/base/base.h"
#include "pjit/mir/cfg/control-flow-graph.h"
#include "pjit/mir/cfg/sequential.h"

namespace pjit {

namespace hir {
class LoopStatementBuilder;
}

namespace mir {

class Context;
class BasicBlockVisitor;
class ControlFlowGraphVisitor;


// Represents a single IF+ELSE control-flow graph. The structure begins with a
// basic block containing the instructions that compute the condition of the
// IF statement. Then, there are two sequential control-flow graphs, containing
// the entry basic blocks to the THEN and ELSE branches, respectively. A pointer
// to a successor control-flow graph must be supplied ahead-of-time as a
// continuation to the THEN and ELSE branches.
class LoopControlFlowGraph : public ControlFlowGraph {
 public:
  LoopControlFlowGraph(Context *context, ControlFlowGraph *parent,
                       SequentialControlFlowGraph *succ);

  virtual ~LoopControlFlowGraph(void) = default;

  virtual void VisitPreOrder(ControlFlowGraphVisitor *);
  virtual void VisitPostOrder(ControlFlowGraphVisitor *);

 protected:
  virtual void DoVisitPreOrder(ControlFlowGraphVisitor *visitor);
  virtual void DoVisitPostOrder(ControlFlowGraphVisitor *visitor);
  virtual void VisitFirst(BasicBlockVisitor *);
  virtual void VisitPredecessor(ControlFlowGraph *, BasicBlockVisitor *);

 private:
  friend class hir::LoopStatementBuilder;

  // The initialization, condition, and update blocks. Initialization also
  // acts as a loop pre-header.
  SequentialControlFlowGraph init;
  SequentialControlFlowGraph condition;
  SequentialControlFlowGraph update;
  SequentialControlFlowGraph body;

  // The symbol that all code in the `if_true` and `if_false` branches are
  // control-dependent on.
  const Symbol *conditional_value;

  // The successor CFG to this IF/ELSE statement.
  SequentialControlFlowGraph *successor;

  PJIT_DISALLOW_COPY_AND_ASSIGN(LoopControlFlowGraph);
};

}  // namespace mir
}  // namespace pjit

#endif  // PJIT_MIR_CFG_LOOP_H_
