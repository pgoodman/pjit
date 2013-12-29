/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * gc.cc
 *
 *  Created on: 2013-12-29
 *      Author: Peter Goodman
 */

#include "pjit/mir/gc.h"
#include "pjit/mir/context.h"
#include "pjit/mir/cfg/basic-block.h"
#include "pjit/mir/cfg/multi-way-branch.h"

namespace pjit {
namespace mir {


GarbageCollectionVisitor::GarbageCollectionVisitor(Context *context_)
    : context(context_) {}


void GarbageCollectionVisitor::VisitPreOrder(SequentialControlFlowGraph *cfg) {
  if (context->seq_allocator.OwnsObject(cfg)) {
    context->seq_allocator.MarkReachable(cfg);
  }
  this->ControlFlowGraphVisitor::VisitPreOrder(cfg);
}


void GarbageCollectionVisitor::VisitPreOrder(ConditionalControlFlowGraph *cfg) {
  if (context->cond_allocator.OwnsObject(cfg)) {
    context->cond_allocator.MarkReachable(cfg);
  }
  this->ControlFlowGraphVisitor::VisitPreOrder(cfg);
}


void GarbageCollectionVisitor::VisitPreOrder(
    MultiWayBranchControlFlowGraph *cfg) {
  if (context->mbr_allocator.OwnsObject(cfg)) {
    context->mbr_allocator.MarkReachable(cfg);
  }
  for (MultiWayBranchArm *arm(cfg->arms); nullptr != arm; arm = arm->next) {
    context->mbr_arm_allocator.MarkReachable(arm);
  }
  this->ControlFlowGraphVisitor::VisitPreOrder(cfg);
}


void GarbageCollectionVisitor::VisitPreOrder(LoopControlFlowGraph *cfg) {
  if (context->loop_allocator.OwnsObject(cfg)) {
    context->loop_allocator.MarkReachable(cfg);
  }
  this->ControlFlowGraphVisitor::VisitPreOrder(cfg);
}


void GarbageCollectionVisitor::VisitPreOrder(BasicBlock *bb) {
  for (Instruction *in(bb->first); nullptr != in; in = in->next) {
    context->instruction_allocator.MarkReachable(in);
    for (unsigned i(0); i < Instruction::kMaxNumOperands; ++i) {
      if (in->operands[i]) {
        context->symbol_allocator.MarkReachable(in->operands[i]);
      }
    }
  }
}

}  // namespace mir
}  // namespace pjit


