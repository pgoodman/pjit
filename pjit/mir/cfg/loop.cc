/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * loop.cc
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#include "pjit/mir/cfg/finder.h"
#include "pjit/mir/cfg/loop.h"

namespace pjit {
namespace mir {


LoopControlFlowGraph::LoopControlFlowGraph(
    Context *context_, ControlFlowGraph *parent_,
    SequentialControlFlowGraph *successor_)
    : ControlFlowGraph(context_, parent_),
      init(context_, this),
      condition(context_, this),
      update(context_, this),
      body(context_, this),
      conditional_value(nullptr),
      successor(successor_) {

  init.successor = &condition;
  body.successor = &update;
  update.successor = &condition;
}


void LoopControlFlowGraph::VisitPreOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  condition.last_visitor = visitor;
  visitor->VisitPreOrder(&init);

  do {  // Visit the body. This will also trigger visiting of the update block.
    PredecessorBasicBlockFinder predecessors(nullptr, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessors);

    visitor->VisitPreOrder(&body);
    PJIT_UNUSED(predecessor_chain);
  } while (0);

  condition.last_visitor = nullptr;

  do {
    // Successors of the condition are either the loop's successor, or the
    // first basic block in the body.
    FirstBasicBlockFinder successors({&body, successor});
    BasicBlockFinderChain successor_chain(
        visitor->find_successors, successors);

    // Predecessors of the condition are the initialization routine and the
    // update routine.
    PredecessorBasicBlockFinder predecessors(&condition, {&init, &update});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessors);

    visitor->VisitPreOrder(&condition);
    PJIT_UNUSED(predecessor_chain);
    PJIT_UNUSED(successor_chain);
  } while (0);

  do {  // Visit the successor of the loop.
    PredecessorBasicBlockFinder predecessor(successor, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPreOrder(successor);
    PJIT_UNUSED(predecessor_chain);
  } while (0);
}


void LoopControlFlowGraph::VisitPostOrder(
    ControlFlowGraphVisitor *visitor) {
  if (!ShouldVisit(visitor)) {
    return;
  }

  do {  // Visit the successor of the loop.
    PredecessorBasicBlockFinder predecessor(successor, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessor);
    visitor->VisitPreOrder(successor);
    PJIT_UNUSED(predecessor_chain);
  } while (0);

  do {
    // Successors of the condition are either the loop's successor, or the
    // first basic block in the body.
    FirstBasicBlockFinder successors({&body, successor});
    BasicBlockFinderChain successor_chain(
        visitor->find_successors, successors);

    // Predecessors of the condition are the initialization routine and the
    // update routine.
    PredecessorBasicBlockFinder predecessors(&condition, {&init, &update});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessors);

    visitor->VisitPreOrder(&condition);
    PJIT_UNUSED(predecessor_chain);
    PJIT_UNUSED(successor_chain);
  } while (0);

  do {  // Visit the body. This will also trigger visiting of the update block.
    PredecessorBasicBlockFinder predecessors(nullptr, {&condition});
    BasicBlockFinderChain predecessor_chain(
        visitor->find_predecessors, predecessors);

    visitor->VisitPreOrder(&body);
    PJIT_UNUSED(predecessor_chain);
  } while (0);

  visitor->VisitPreOrder(&init);
}


PJIT_DEFINE_CFG_VISITOR_RESOLVER(LoopControlFlowGraph)


void LoopControlFlowGraph::VisitFirst(BasicBlockVisitor *visitor) {
  init.VisitFirst(visitor);
}


void LoopControlFlowGraph::VisitPredecessor(ControlFlowGraph *search,
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
