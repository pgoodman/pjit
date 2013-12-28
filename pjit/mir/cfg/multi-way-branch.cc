/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * multi-way-branch.cc
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#include "pjit/mir/cfg/multi-way-branch.h"
#include "pjit/mir/cfg/finder.h"

namespace pjit {
namespace mir {

MultiWayBranchArm::MultiWayBranchArm(Context *context,
                                     ControlFlowGraph *parent,
                                     const Symbol *value_,
                                     MultiWayBranchArm *next_,
                                     ControlFlowGraph *successor)
    : value(value_),
      if_true(context, parent),
      next(next_) {
  if_true.successor = successor;
}


MultiWayBranchControlFlowGraph::MultiWayBranchControlFlowGraph(
    Context *context,
    ControlFlowGraph *parent,
    SequentialControlFlowGraph *succ)
    : ControlFlowGraph(context, parent),
      condition(context, this),
      conditional_value(nullptr),
      arms(nullptr),
      successor(succ) {}


void MultiWayBranchControlFlowGraph::VisitPreOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  do { // Visit the condition; predecessors are inherited.
    MultiWayFirstBasicBlockFinder successors(arms);
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

  // Visit each arm of the multi-way branch.
  for (MultiWayBranchArm *arm(arms); nullptr != arm; arm = arm->next) {
    PredecessorBasicBlockFinder predecessor(nullptr, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPreOrder(&(arm->if_true));
    PJIT_UNUSED(predecessor_chain);
  }

  // The successor of the conditional CFG wasn't previously visited, so now
  // we can go and visit it.
  if (!successor_was_visited) {
    successor->last_visitor = nullptr;
    MultiWayPredecessorBasicBlockFinder predecessor(successor, arms);
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPreOrder(successor);
    PJIT_UNUSED(predecessor_chain);
  }
}


void MultiWayBranchControlFlowGraph::VisitPostOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  // Visit the successor of the condition. Unlike with a pre-order traversal,
  // we don't need to play around with the visitor id stuff because we're seeing
  // the successor first.
  do {
    MultiWayPredecessorBasicBlockFinder predecessor(successor, arms);
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPostOrder(successor);
    PJIT_UNUSED(predecessor_chain);
  } while (0);

  // Visit the `if_true` and `if_false` branches.
  for (MultiWayBranchArm *arm(arms); nullptr != arm; arm = arm->next) {
    PredecessorBasicBlockFinder predecessor(nullptr, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPostOrder(&(arm->if_true));
    PJIT_UNUSED(predecessor_chain);
  }

  do { // Visit the condition; predecessors are inherited.
    MultiWayFirstBasicBlockFinder successors(arms);
    BasicBlockFinderChain successor_chain(
        visitor->find_successors, successors);
    visitor->VisitPostOrder(&condition);
    PJIT_UNUSED(successor_chain);
  } while (0);
}


void MultiWayBranchControlFlowGraph::VisitFirst(BasicBlockVisitor *visitor) {
  visitor->Visit(&(condition.bb));
}


void MultiWayBranchControlFlowGraph::VisitPredecessor(
    ControlFlowGraph *search, BasicBlockVisitor *visitor) {
  successor->VisitPredecessor(search, visitor);
}

}  // namespace mir
}  // namespace pjit
