/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * context.cc
 *
 *  Created on: 2013-12-21
 *      Author: Peter Goodman
 */

#include "pjit/mir/context.h"
#include "pjit/mir/instruction.h"
#include "pjit/mir/gc.h"

namespace pjit {
namespace mir {


Context::Context(void)
    : next_symbol_id(1),
      entry(this, nullptr),
      exit(this, &entry),
      current(&entry),
      if_builder(nullptr),
      mbr_builder(nullptr) {
  entry.successor = &exit;
}


Symbol *Context::MakeSymbol(const TypeInfo *type) {
  return symbol_allocator.Allocate(type, nullptr, next_symbol_id++);
}


Symbol *Context::MakeSymbol(const TypeInfo *type, const char *name) {
  return symbol_allocator.Allocate(type, name, next_symbol_id++);
}



Symbol *Context::CopySymbol(const Symbol *that) {
  return symbol_allocator.Allocate(that->type, that->value.name, that->id);
}


void Context::EmitInstruction(Operation op,
                              std::initializer_list<const Symbol *> args) {
  current->Append(instruction_allocator.Allocate(op, args));
}


const Symbol *Context::EmitConvertType(const TypeInfo *to_type,
                                       const Symbol *from_value) {
  if (from_value->type == to_type) {
    return from_value;
  }

  const Symbol *dest(MakeSymbol(to_type));
  EmitInstruction(Operation::OP_CONVERT_TYPE, {dest, from_value});
  return dest;
}


void Context::VisitPreOrder(ControlFlowGraphVisitor *visitor) {
  visitor->VisitPreOrder(&entry);
}


void Context::VisitPostOrder(ControlFlowGraphVisitor *visitor) {
  visitor->VisitPostOrder(&entry);
}


void Context::GarbageCollect(void) {
  symbol_allocator.MarkAllUnreachable();
  instruction_allocator.MarkAllUnreachable();
  seq_allocator.MarkAllUnreachable();
  cond_allocator.MarkAllUnreachable();
  mbr_allocator.MarkAllUnreachable();
  mbr_arm_allocator.MarkAllUnreachable();
  loop_allocator.MarkAllUnreachable();

  GarbageCollectionVisitor gc_visitor(this);
  VisitPreOrder(&gc_visitor);

  symbol_allocator.FreeUnreachable();
  instruction_allocator.FreeUnreachable();
  seq_allocator.FreeUnreachable();
  cond_allocator.FreeUnreachable();
  mbr_allocator.FreeUnreachable();
  mbr_arm_allocator.FreeUnreachable();
  loop_allocator.FreeUnreachable();
}


// Link a successor into the CFG.
void Context::LinkSuccessor(SequentialControlFlowGraph *successor) {
  if (current) {
    successor->successor = current->successor;
    if (successor->successor &&
        successor->successor->parent == current) {
      successor->successor->parent = successor;
    }
  }
}


// Link the new current CFG into the context.
void Context::LinkCurrent(SequentialControlFlowGraph *new_current,
                          ControlFlowGraph *linked_successor) {
  if (current) {
    current->successor = linked_successor;
  }
  current = new_current;
}

}  // namespace mir
}  // namespace pjit

