/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * conditional.cc
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#include "pjit/mir/cfg/finder.h"
#include "pjit/mir/cfg/conditional.h"

namespace pjit {
namespace mir {


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
  }

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
  }

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


}  // namespace mir
}  // namespace pjit
