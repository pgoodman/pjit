/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * control-flow-graph.cc
 *
 *  Created on: 2013-12-25
 *      Author: Peter Goodman
 */

#include <initializer_list>

#include "pjit/base/libc.h"

#include "pjit/mir/control-flow-graph.h"
#include "pjit/mir/context.h"
#include "pjit/mir/instruction.h"


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


// Defines a dynamically scoped chain of basic block finders. This class is
// responsible for saving a particular block finder, updating it, and then
// restoring the saved finder on destruction.
class BasicBlockFinderChain {
 public:
  BasicBlockFinderChain(BasicBlockFinder *&ptr_ref, BasicBlockFinder &curr)
     : ptr(&ptr_ref),
       prev_value(ptr_ref) {
    *ptr = &curr;
  }

  ~BasicBlockFinderChain(void) {
    *ptr = prev_value;
  }

 private:
  BasicBlockFinder **ptr;
  BasicBlockFinder *prev_value;

  BasicBlockFinderChain(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlockFinderChain);
};


// A generic basic block finder for finding some set of basic blocks.
class BasicBlockFinder {
 public:
  explicit BasicBlockFinder(std::initializer_list<ControlFlowGraph *> args) {
    memset(&(cfgs[0]), 0, kMaxNumCfgs * sizeof(ControlFlowGraph *));
    memcpy(&(cfgs[0]), args.begin(), args.size() * sizeof(ControlFlowGraph *));
  }

  virtual ~BasicBlockFinder(void) = default;
  virtual void Visit(BasicBlockVisitor *visitor) = 0;

 protected:
  enum {
    kMaxNumCfgs = 2
  };

  ControlFlowGraph *cfgs[kMaxNumCfgs];

 private:
  BasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlockFinder);
};


// A basic block finder that, given a control flow graph, finds the entry basic
// blocks to the control flow graphs and invokes a visitor on those entry
// blocks.
class FirstBasicBlockFinder : public BasicBlockFinder {
 public:
  explicit FirstBasicBlockFinder(
      std::initializer_list<ControlFlowGraph *> args)
      : BasicBlockFinder(args) {}

  virtual ~FirstBasicBlockFinder(void) = default;

  virtual void Visit(BasicBlockVisitor *visitor) {
    for (unsigned i(0); i < kMaxNumCfgs; ++i) {
      if (cfgs[i]) {
        cfgs[i]->VisitFirst(visitor);
      }
    }
  }

 private:
  FirstBasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(FirstBasicBlockFinder);
};


// A basic block finder that visits the predecessors of a specific control-flow
// graph.
class PredecessorBasicBlockFinder : public BasicBlockFinder {
 public:
  explicit PredecessorBasicBlockFinder(
      ControlFlowGraph *search_,
      std::initializer_list<ControlFlowGraph *> args)
      : BasicBlockFinder(args),
        search(search_) {}

  virtual ~PredecessorBasicBlockFinder(void) = default;

  virtual void Visit(BasicBlockVisitor *visitor) {
    for (unsigned i(0); i < kMaxNumCfgs; ++i) {
      if (cfgs[i]) {
        cfgs[i]->VisitPredecessor(search, visitor);
      }
    }
  }

 private:
  ControlFlowGraph *search;

  PredecessorBasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(PredecessorBasicBlockFinder);
};


ControlFlowGraph::ControlFlowGraph(Context *context_, ControlFlowGraph *parent_)
    : context(context_),
      parent(parent_),
      last_visitor(nullptr) {}


SequentialControlFlowGraph::SequentialControlFlowGraph(
    Context *context_, ControlFlowGraph *parent_)
    : ControlFlowGraph(context_, parent_),
      bb(),
      successor(nullptr) {}


void SequentialControlFlowGraph::VisitPreOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  // If there is no successor, then the successors are inherited from the
  // parent CFG. The predecessors are unconditionally inherited from the
  // parent CFG.
  if (successor) {
    FirstBasicBlockFinder successors({successor});
    BasicBlockFinderChain successor_chain(
        visitor->find_successors, successors);
    visitor->VisitPreOrder(&bb);
    PJIT_UNUSED(successor_chain);
  } else {
    visitor->VisitPreOrder(&bb);
  }

  if (successor) {  // Define predecessor inheritance for children.
    FirstBasicBlockFinder predecessor({this});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    successor->VisitPreOrder(visitor);
    PJIT_UNUSED(predecessor_chain);
  }
}


void SequentialControlFlowGraph::VisitPostOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  if (successor) {  // Define predecessor inheritance for children.
    FirstBasicBlockFinder predecessor({this});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    successor->VisitPreOrder(visitor);
    PJIT_UNUSED(predecessor_chain);
  }

  // If there is no successor, then the successors are inherited from the
  // parent CFG. The predecessors are unconditionally inherited from the
  // parent CFG.
  if (successor) {
    FirstBasicBlockFinder successors({successor});
    BasicBlockFinderChain successor_chain(
        visitor->find_successors, successors);
    visitor->VisitPreOrder(&bb);
    PJIT_UNUSED(successor_chain);
  } else {
    visitor->VisitPreOrder(&bb);
  }
}


void SequentialControlFlowGraph::VisitFirst(BasicBlockVisitor *visitor) {
  visitor->Visit(&bb);
}


void SequentialControlFlowGraph::VisitPredecessor(ControlFlowGraph *search,
                                                  BasicBlockVisitor *visitor) {
  if (successor == search) {
    visitor->Visit(&(bb));
  } else {
    successor->VisitPredecessor(search, visitor);
  }
}


ConditionalControlFlowGraph::ConditionalControlFlowGraph(
    Context *context_, ControlFlowGraph *parent_,
    SequentialControlFlowGraph *successor_)
    : ControlFlowGraph(context_, parent_),
      condition(context_, this),
      conditional_value(nullptr),
      if_true(context_, this),
      if_false(context_, this),
      successor(successor_) {
  if_true.successor = successor;
  if_false.successor = successor;
}


void ConditionalControlFlowGraph::VisitPreOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  do { // Visit the condition; predecessors are inherited.
    FirstBasicBlockFinder successors({&if_true, &if_false});
    BasicBlockFinderChain successor_chain(
        visitor->find_successors, successors);
    visitor->VisitPreOrder(&condition);
    PJIT_UNUSED(successor_chain);
  } while (0);

  // If we haven't already visited the successor of the control-flow graph
  // then make sure that we mark it as already visited so that we can visit
  // it later.
  //
  // TODO(pag): Assert that the successor was not already visited.
  bool successor_was_visited(true);
  if (successor->last_visitor != visitor) {
    successor_was_visited = false;
    successor->last_visitor = visitor;
  }

  // Visit the `if_true` and `if_false` branches.
  for (SequentialControlFlowGraph *branch : {&if_true, &if_false}) {
    PredecessorBasicBlockFinder predecessor(nullptr, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPreOrder(branch);
    PJIT_UNUSED(predecessor_chain);
  } while (0);

  // The successor of the conditional CFG wasn't previously visited, so now
  // we can go and visit it.
  if (!successor_was_visited) {
    successor->last_visitor = nullptr;
    PredecessorBasicBlockFinder predecessor(successor, {&if_true, &if_false});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPreOrder(successor);
    PJIT_UNUSED(predecessor_chain);
  }
}


void ConditionalControlFlowGraph::VisitPostOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  // Visit the successor of the condition. Unlike with a pre-order traversal,
  // we don't need to play around with the visitor id stuff because we're seeing
  // the successor first.
  do {
    PredecessorBasicBlockFinder predecessor(successor, {&if_true, &if_false});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPostOrder(successor);
    PJIT_UNUSED(predecessor_chain);
  } while (0);

  // Visit the `if_true` and `if_false` branches.
  for (SequentialControlFlowGraph *branch : {&if_true, &if_false}) {
    PredecessorBasicBlockFinder predecessor(nullptr, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPostOrder(branch);
    PJIT_UNUSED(predecessor_chain);
  } while (0);

  do { // Visit the condition; predecessors are inherited.
    FirstBasicBlockFinder successors({&if_true, &if_false});
    BasicBlockFinderChain successor_chain(
        visitor->find_successors, successors);
    visitor->VisitPostOrder(&condition);
    PJIT_UNUSED(successor_chain);
  } while (0);
}


void ConditionalControlFlowGraph::VisitFirst(BasicBlockVisitor *visitor) {
  condition.VisitFirst(visitor);
}


void ConditionalControlFlowGraph::VisitPredecessor(ControlFlowGraph *search,
                                                   BasicBlockVisitor *visitor) {
  // Note: We special case this function to jump straight to the successor node
  //       because we know how this function is being used (by the
  //       `PredecessorBasicBlockFinder` to find the predecessor basic blocks
  //       of a condition or loop's exit block, and so if we're in here, then
  //       this means that we're inspecting an condition/loop that's internal to
  //       the one we originally operated on.
  successor->VisitPredecessor(search, visitor);
}


ControlFlowGraphVisitor::ControlFlowGraphVisitor(void)
    : find_successors(nullptr),
      find_predecessors(nullptr) {}


#define PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(cls) \
  void ControlFlowGraphVisitor::VisitPreOrder(cls *cfg) { \
    if (cfg) { \
      cfg->VisitPreOrder(this); \
    } \
  } \
  void ControlFlowGraphVisitor::VisitPostOrder(cls *cfg) { \
    if (cfg) { \
      cfg->VisitPostOrder(this); \
    } \
  }


PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(SequentialControlFlowGraph)
PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(ConditionalControlFlowGraph)

void ControlFlowGraphVisitor::VisitSuccessors(BasicBlockVisitor *visitor) {
  if (find_successors) {
    find_successors->Visit(visitor);
  }
}


void ControlFlowGraphVisitor::VisitPredecessors(BasicBlockVisitor *visitor) {
  if (find_predecessors) {
    find_predecessors->Visit(visitor);
  }
}

}  // namespace mir
}  // namespace pjit
