/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * multi-way-branch.h
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CFG_MULTI_WAY_BRANCH_H_
#define PJIT_MIR_CFG_MULTI_WAY_BRANCH_H_

#include "pjit/mir/cfg/control-flow-graph.h"
#include "pjit/mir/cfg/sequential.h"

namespace pjit {
namespace hir {
class SwitchStatementBuilder;
class CaseStatementBuilder;
}

namespace mir {

class Symbol;
class Context;
class BasicBlockVisitor;
class ControlFlowGraphVisitor;
class MultiWayBranchControlFlowGraph;
class MultiWayPredecessorBasicBlockFinder;
class MultiWayFriendBasicBlockFinder;
class GarbageCollectionVisitor;

// Represents a single arm of a multi-way branch CFG.
class MultiWayBranchArm {
 public:
  explicit MultiWayBranchArm(Context *context,
                             ControlFlowGraph *parent,
                             const Symbol *value_,
                             MultiWayBranchArm *next_,
                             ControlFlowGraph *successor);

 private:
  friend class hir::CaseStatementBuilder;
  friend class MultiWayBranchControlFlowGraph;
  friend class MultiWayFirstBasicBlockFinder;
  friend class MultiWayPredecessorBasicBlockFinder;
  friend class GarbageCollectionVisitor;

  // The value that the switch condition value must equal to in order to take
  // this arm of the multi-way branch.
  const Symbol * const value;

  // If `value` equals `MultiWayBranchControlFlowGraph::conditional_value` then
  // control flows into this CFG.
  SequentialControlFlowGraph if_true;

  // The next arm of the branch, or NULL if there are no other arms.
  MultiWayBranchArm *next;

  MultiWayBranchArm(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(MultiWayBranchArm);
};


// Represents a single IF+ELSE control-flow graph. The structure begins with a
// basic block containing the instructions that compute the condition of the
// IF statement. Then, there are two sequential control-flow graphs, containing
// the entry basic blocks to the THEN and ELSE branches, respectively. A pointer
// to a successor control-flow graph must be supplied ahead-of-time as a
// continuation to the THEN and ELSE branches.
class MultiWayBranchControlFlowGraph : public ControlFlowGraph {
 public:
  MultiWayBranchControlFlowGraph(Context *context, ControlFlowGraph *parent,
                                 SequentialControlFlowGraph *succ);

  virtual ~MultiWayBranchControlFlowGraph(void) = default;

  virtual void VisitPreOrder(ControlFlowGraphVisitor *);
  virtual void VisitPostOrder(ControlFlowGraphVisitor *);

 protected:
  virtual void DoVisitPreOrder(ControlFlowGraphVisitor *visitor);
  virtual void DoVisitPostOrder(ControlFlowGraphVisitor *visitor);
  virtual void VisitFirst(BasicBlockVisitor *);
  virtual void VisitPredecessor(ControlFlowGraph *, BasicBlockVisitor *);

 private:
  friend class hir::SwitchStatementBuilder;
  friend class hir::CaseStatementBuilder;
  friend class Context;
  friend class MultiWayFirstBasicBlockFinder;
  friend class MultiWayPredecessorBasicBlockFinder;
  friend class GarbageCollectionVisitor;

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
  MultiWayBranchArm *arms;
  MultiWayBranchArm *default_arm;

  // The successor CFG to this IF/ELSE statement.
  SequentialControlFlowGraph *successor;

  MultiWayBranchControlFlowGraph(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(MultiWayBranchControlFlowGraph);
};

}  // namespace mir
}  // namespace pjit

#endif  // PJIT_MIR_CFG_MULTI_WAY_BRANCH_H_
