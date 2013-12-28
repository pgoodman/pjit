/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * hir-to-mir.cc
 *
 *  Created on: 2013-12-26
 *      Author: Peter Goodman
 */

#include "pjit/hir/hir-to-mir.h"

namespace pjit {
namespace hir {

ElseStatementBuilder::ElseStatementBuilder(mir::Context &context) {
  context.current = &(context.if_builder->conditional->if_false);
}


IfStatementBuilder::IfStatementBuilder(mir::Context &context_)
    : context(&context_),
      conditional(nullptr),
      successor(nullptr),
      saved_if_builder(context->if_builder)
{
  // Make and link a new successor in-between the current CFG's old successor
  // and our conditional CFG.
  successor = context->seq_allocator.Allocate(context, context->current);
  context->LinkSuccessor(successor);

  // Make and link a new conditional CFG into the successor chain.
  conditional = context->cond_allocator.Allocate(
      context,
      context->current,
      successor);

  context->LinkCurrent(&(conditional->condition), conditional);
  context->if_builder = this;
}


IfStatementBuilder::~IfStatementBuilder(void) {
  context->current = successor;
  context->if_builder = saved_if_builder;
}


CaseStatementBuilder::CaseStatementBuilder(mir::Context &context_,
                                           const Symbol *symbol)
    : context(&context_) {
  mir::MultiWayBranchArm *arm(context->mbr_arm_allocator.Allocate(
      context,
      context->mbr_builder->mbr,  // Parent node of the CASE.
      symbol,  // Symbol that must match the switch `condition_value`.
      context->mbr_builder->mbr->arms,  // Next arm; add to head of `arms` list.
      context->mbr_builder->mbr->successor));  // Fall-through successor.

  context->mbr_builder->mbr->arms = arm;
  context->current = &(arm->if_true);
}


CaseStatementBuilder::~CaseStatementBuilder(void) {
  // Ensure that emitting instructions without a CASE statement results in
  // a fault.
  context->current = nullptr;
}


SwitchStatementBuilder::SwitchStatementBuilder(mir::Context &context_)
    : context(&context_),
      mbr(nullptr),
      successor(nullptr),
      saved_mbr_builder(context->mbr_builder)
{
  // Make and link a new successor in-between the current CFG's old successor
  // and our conditional CFG.
  successor = context->seq_allocator.Allocate(context, context->current);
  context->LinkSuccessor(successor);

  // Make and link a new conditional CFG into the successor chain.
  mbr = context->mbr_allocator.Allocate(
      context, context->current, successor);

  context->LinkCurrent(&(mbr->condition), mbr);
  context->mbr_builder = this;
}


SwitchStatementBuilder::~SwitchStatementBuilder(void) {
  context->current = successor;
  context->mbr_builder = saved_mbr_builder;
}

}  // namespace hir
}  // namespace pjit
