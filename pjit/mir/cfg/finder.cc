/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * finder.cc
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#include "pjit/base/libc.h"
#include "pjit/mir/cfg/finder.h"
#include "pjit/mir/cfg/control-flow-graph.h"
#include "pjit/mir/cfg/multi-way-branch.h"

namespace pjit {
namespace mir {


BasicBlockFinderChain::BasicBlockFinderChain(BasicBlockFinder *&ptr_ref,
                                             BasicBlockFinder &curr)
   : ptr(&ptr_ref),
     prev_value(ptr_ref) {
  *ptr = &curr;
}


BasicBlockFinderChain::~BasicBlockFinderChain(void) {
  *ptr = prev_value;
}


ArrayBasicBlockFinder::ArrayBasicBlockFinder(
    std::initializer_list<ControlFlowGraph *> args) {
  memset(&(cfgs[0]), 0, kMaxNumCfgs * sizeof(ControlFlowGraph *));
  memcpy(&(cfgs[0]), args.begin(), args.size() * sizeof(ControlFlowGraph *));
}


FirstBasicBlockFinder::FirstBasicBlockFinder(
    std::initializer_list<ControlFlowGraph *> args)
    : ArrayBasicBlockFinder(args) {}


void FirstBasicBlockFinder::Visit(BasicBlockVisitor *visitor) {
  for (unsigned i(0); i < kMaxNumCfgs; ++i) {
    if (cfgs[i]) {
      cfgs[i]->VisitFirst(visitor);
    }
  }
}


PredecessorBasicBlockFinder::PredecessorBasicBlockFinder(
    ControlFlowGraph *search_,
    std::initializer_list<ControlFlowGraph *> args)
    : ArrayBasicBlockFinder(args),
      search(search_) {}


void PredecessorBasicBlockFinder::Visit(BasicBlockVisitor *visitor) {
  for (unsigned i(0); i < kMaxNumCfgs; ++i) {
    if (cfgs[i]) {
      cfgs[i]->VisitPredecessor(search, visitor);
    }
  }
}


MultiWayFirstBasicBlockFinder::MultiWayFirstBasicBlockFinder(
    MultiWayBranchArm *arms_)
    : arms(arms_) {}


void MultiWayFirstBasicBlockFinder::Visit(BasicBlockVisitor *visitor) {
  for (MultiWayBranchArm *arm(arms); nullptr != arm; arm = arm->next) {
    arm->if_true.VisitFirst(visitor);
  }
}


MultiWayPredecessorBasicBlockFinder::MultiWayPredecessorBasicBlockFinder(
    ControlFlowGraph *search_, MultiWayBranchArm *arms_)
    : search(search_),
      arms(arms_) {}


void MultiWayPredecessorBasicBlockFinder::Visit(BasicBlockVisitor *visitor) {
  for (MultiWayBranchArm *arm(arms); nullptr != arm; arm = arm->next) {
    arm->if_true.VisitPredecessor(search, visitor);
  }
}

}  // namespace mir
}  // namespace pjit
