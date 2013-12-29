/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * control-flow-graph.h
 *
 *  Created on: 2013-12-23
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CFG_CONTROL_FLOW_GRAPH_H_
#define PJIT_MIR_CFG_CONTROL_FLOW_GRAPH_H_

#include "pjit/base/base.h"

namespace pjit {

// Forward declarations. Needed for conversion from HIR to MIR.
class Symbol;

namespace mir {

// Forward declarations. Needed because there is a fair amount of sharing going
// on between various classes so that things can get properly linked together
// and initialized.
class Context;
class BasicBlock;
class SequentialControlFlowGraph;
class ConditionalControlFlowGraph;
class MultiWayBranchControlFlowGraph;
class LoopControlFlowGraph;
class ControlFlowGraphVisitor;
class BasicBlockVisitor;
class BasicBlockFinder;
class FirstBasicBlockFinder;
class PredecessorBasicBlockFinder;


// Represents an abstract control-flow graph. Every control-flow graph is
// managed by a `Context` object, which is responsible for allocating all
// runtime memory. Every control-flow graph has a parent control-flow graph.
class ControlFlowGraph {
 public:
  ControlFlowGraph(Context *context, ControlFlowGraph *parent);
  virtual ~ControlFlowGraph(void) = default;

  virtual void VisitPreOrder(ControlFlowGraphVisitor *) = 0;
  virtual void VisitPostOrder(ControlFlowGraphVisitor *) = 0;

 protected:
  Context * const context;
  ControlFlowGraph *parent;
  ControlFlowGraphVisitor *last_visitor;

  virtual void DoVisitPreOrder(ControlFlowGraphVisitor *) = 0;
  virtual void DoVisitPostOrder(ControlFlowGraphVisitor *) = 0;

  // Visit the first basic block(s) within this CFG.
  virtual void VisitFirst(BasicBlockVisitor *) = 0;

  // Treat this CFG as containing the predecessor basic block(s) to another
  // basic block, and go and visit those predecessors.
  virtual void VisitPredecessor(ControlFlowGraph *, BasicBlockVisitor *) = 0;

  // Check whether or not we should visit the current CFG.
  inline bool ShouldVisit(ControlFlowGraphVisitor *visitor) {
    if (last_visitor == visitor) {
      return false;
    }
    last_visitor = visitor;
    return true;
  }

 private:
  friend class Context;
  friend class SequentialControlFlowGraph;
  friend class MultiWayBranchControlFlowGraph;
  friend class FirstBasicBlockFinder;
  friend class PredecessorBasicBlockFinder;

  ControlFlowGraph(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ControlFlowGraph);
};


#define PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(cls) \
  friend class cls; \
  virtual void VisitPreOrder(cls *cfg); \
  virtual void VisitPostOrder(cls *cfg)


#define PJIT_DEFINE_CFG_VISITOR_RESOLVER(cls) \
  void cls::DoVisitPreOrder( \
      ControlFlowGraphVisitor *visitor) { \
    visitor->VisitPreOrder(this); \
  } \
  void cls::DoVisitPostOrder( \
      ControlFlowGraphVisitor *visitor) { \
    visitor->VisitPostOrder(this); \
  }


// Abstract control-flow graph visitor. Two traversal orders are defined over
// control-flow graphs: pre-order traversals and post-order traversals.
class ControlFlowGraphVisitor {
 public:
  ControlFlowGraphVisitor(void);
  virtual ~ControlFlowGraphVisitor(void) = default;

  PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(SequentialControlFlowGraph);
  PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(ConditionalControlFlowGraph);
  PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(MultiWayBranchControlFlowGraph);
  PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(LoopControlFlowGraph);

  virtual void VisitPreOrder(BasicBlock *) {}
  virtual void VisitPostOrder(BasicBlock *) {}

 protected:
  virtual void VisitSuccessors(BasicBlockVisitor *);
  virtual void VisitPredecessors(BasicBlockVisitor *);

 private:
  // Finders used to discover the successors and predecessors of the currently
  // visited basic blocks. Successors and predecessors are defined structurally
  // and are not stored in any single container on a per-basic block basis.
  BasicBlockFinder *find_successors;
  BasicBlockFinder *find_predecessors;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ControlFlowGraphVisitor);
};


#undef PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS


}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_CFG_CONTROL_FLOW_GRAPH_H_
