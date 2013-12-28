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
  successor = context->scfg_allocator.Allocate(context, context->current);
  if (context->current) {
    successor->successor = context->current->successor;
    if (successor->successor &&
        successor->successor->parent == context->current) {
      successor->successor->parent = successor;
    }
  }

  // Make and link a new conditional CFG into the successor chain.
  conditional = context->ccfg_allocator.Allocate(
      context, context->current, successor);
  if (context->current) {
    context->current->successor = conditional;
  }
  context->current = &(conditional->condition);
  context->if_builder = this;
}


IfStatementBuilder::~IfStatementBuilder(void) {
  context->current = successor;
  context->if_builder = saved_if_builder;
}


}  // namespace hir
}  // namespace pjit
