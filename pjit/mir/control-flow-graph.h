/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * control-flow-graph.h
 *
 *  Created on: 2013-12-23
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CONTROL_FLOW_GRAPH_H_
#define PJIT_MIR_CONTROL_FLOW_GRAPH_H_


#include "pjit/base/base.h"


namespace pjit {

// Forward declarations. Needed for conversion from HIR to MIR.
class Symbol;

namespace hir {
class IfStatementBuilder;
class ElseStatementBuilder;
}  // namespace hir

namespace mir {


// Forward declarations. Needed because there is a fair amount of sharing going
// on between various classes so that things can get properly linked together
// and initialized.
class Context;
class Instruction;
class ControlFlowGraph;
class SequentialControlFlowGraph;
class ConditionalControlFlowGraph;
class ControlFlowGraphVisitor;
class BasicBlockVisitor;
class BasicBlockFinder;
class FirstBasicBlockFinder;
class PredecessorBasicBlockFinder;


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
  friend class ConditionalControlFlowGraph;

  BasicBlock(void);

  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};


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
  friend class hir::IfStatementBuilder;
  friend class SequentialControlFlowGraph;
  friend class ConditionalControlFlowGraph;
  friend class FirstBasicBlockFinder;
  friend class PredecessorBasicBlockFinder;

  ControlFlowGraph(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ControlFlowGraph);
};


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
  friend class ControlFlowGraphVisitor;

  BasicBlock bb;
  ControlFlowGraph *successor;

  PJIT_DISALLOW_COPY_AND_ASSIGN(SequentialControlFlowGraph);
};


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
  friend class ControlFlowGraphVisitor;

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


#define PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(cls) \
  friend class cls; \
  virtual void VisitPreOrder(cls *cfg); \
  virtual void VisitPostOrder(cls *cfg)


class BasicBlockVisitor {
 public:
  BasicBlockVisitor(void) = default;
  virtual ~BasicBlockVisitor(void) = default;
  virtual void Visit(BasicBlock *bb) = 0;

 private:
  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlockVisitor);
};


// Represents an abstract control-flow graph visitor. Two traversal orders are
// defined over control-flow graphs: pre-order traversals and post-order
// traversals.
class ControlFlowGraphVisitor {
 public:
  ControlFlowGraphVisitor(void);
  virtual ~ControlFlowGraphVisitor(void) = default;

  PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(SequentialControlFlowGraph);
  PJIT_DECLARE_CONTROL_FLOW_GRAPH_VISITORS(ConditionalControlFlowGraph);

  virtual void VisitPreOrder(BasicBlock *) {}
  virtual void VisitPostOrder(BasicBlock *) {}

 protected:
  virtual void VisitSuccessors(BasicBlockVisitor *);
  virtual void VisitPredecessors(BasicBlockVisitor *);

 private:
  BasicBlockFinder *find_successors;
  BasicBlockFinder *find_predecessors;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ControlFlowGraphVisitor);
};


#undef PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS


}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_CONTROL_FLOW_GRAPH_H_
