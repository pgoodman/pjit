/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * conditional.h
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CFG_CONDITIONAL_H_
#define PJIT_MIR_CFG_CONDITIONAL_H_

#include "pjit/base/base.h"
#include "pjit/mir/cfg/control-flow-graph.h"
#include "pjit/mir/cfg/sequential.h"

namespace pjit {

namespace hir {
class IfStatementBuilder;
class ElseStatementBuilder;
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
class ConditionalControlFlowGraph : public ControlFlowGraph {
 public:
  ConditionalControlFlowGraph(Context *context, ControlFlowGraph *parent,
                              SequentialControlFlowGraph *succ);

  virtual ~ConditionalControlFlowGraph(void) = default;

  virtual void VisitPreOrder(ControlFlowGraphVisitor *);
  virtual void VisitPostOrder(ControlFlowGraphVisitor *);

 protected:
  virtual void VisitFirst(BasicBlockVisitor *);
  virtual void VisitPredecessor(ControlFlowGraph *, BasicBlockVisitor *);

 private:
  friend class hir::IfStatementBuilder;
  friend class hir::ElseStatementBuilder;

  // The control-flow graph containing the condition.
  //
  // Note: This control-flow graph *does not* have a successor set; it is
  //       implied by means of the structure of this class, as it would need
  //       to have two successors. A `SequentialControlFlowGraph` is used
  //       (despite this incongruency) because then it properly integrates with
  //       `Context`'s tracking of the current CFG to append instructions to.
  SequentialControlFlowGraph condition;

  // The symbol that all code in the `if_true` and `if_false` branches are
  // control-dependent on.
  const Symbol *conditional_value;

  // The branch that is taken when the `conditional_value` is true.
  SequentialControlFlowGraph if_true;

  // The branch that is taken when the `conditional_value` is false.
  SequentialControlFlowGraph if_false;

  // The successor CFG to this IF/ELSE statement.
  SequentialControlFlowGraph *successor;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ConditionalControlFlowGraph);
};

}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_CFG_CONDITIONAL_H_
