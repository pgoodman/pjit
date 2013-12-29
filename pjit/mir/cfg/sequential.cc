/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * sequential.cc
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#include "pjit/mir/cfg/finder.h"
#include "pjit/mir/cfg/sequential.h"

namespace pjit {
namespace mir {

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
  } else if (successor) {
    successor->VisitPredecessor(search, visitor);
  }
}

}  // namespace mir
}  // namespace pjit
