/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * context.h
 *
 *  Created on: 2013-12-20
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CONTEXT_H_
#define PJIT_MIR_CONTEXT_H_

#include <initializer_list>

#include "pjit/base/allocate.h"
#include "pjit/base/symbol.h"
#include "pjit/base/numeric-types.h"
#include "pjit/base/type-traits.h"

#include "pjit/mir/instruction.h"
#include "pjit/mir/control-flow-graph.h"


namespace pjit {

namespace hir {
class IfStatementBuilder;
class ElseStatementBuilder;
}  // namespace hir

namespace mir {

// Represents a compilation "context" for the medium-level intermediate
// representation. The MIR has a fairly direct correspondence to C and its
// AST, in that MIR has no explicit control-flow constructs (except for function
// calls), and instead depends on the in-memory tree-like structure of the
// various control-flow graph classes to implicitly define the flow of control.
// The compilation context is primarily responsible for managing allocations.
// MIR compilation contexts are garbage collected using a simple mark and sweep
// collector, implemented inside the `Allocator`.
class Context {
 private:
  friend class hir::IfStatementBuilder;
  friend class hir::ElseStatementBuilder;

  friend class BasicBlock;
  friend class SequentialControlFlowGraph;
  friend class ConditionalControlFlowGraph;

  unsigned next_symbol_id;

  Allocator<Symbol> symbol_allocator;
  Allocator<Instruction> instruction_allocator;
  Allocator<SequentialControlFlowGraph> scfg_allocator;
  Allocator<ConditionalControlFlowGraph> ccfg_allocator;

  // The top-level control-flow graph to which all
  SequentialControlFlowGraph entry;
  SequentialControlFlowGraph exit;

  // The current sequential control-flow graph to which instructions are added.
  SequentialControlFlowGraph *current;

  // The current HIR if-statement builder. When HIR is used, these are
  // automatically arranged into a stack by being saved in the
  // `IfStatementBuilder` constructor and restored in the destructor.
  hir::IfStatementBuilder *if_builder;

 public:

  Context(void);

  Symbol *MakeSymbol(const TypeInfo *type);
  Symbol *MakeSymbol(const TypeInfo *type, const char *name);

  // Immediate constant.
  template <
    typename T,
    typename EnableIf<
      TypesAreEqual<const TypeInfo *, T>::RESULT,
      void,
      int
    >::Type = 0
  >
  Symbol *MakeSymbol(T val) {
    return symbol_allocator.Allocate(val);
  }

  Symbol *CopySymbol(const Symbol *that);

  // Create and emit an instruction to the current basic block.
  void EmitInstruction(Operation op,
                       std::initializer_list<const Symbol *> args);

  const Symbol *EmitConvertType(const TypeInfo *to_type,
                                const Symbol *from_value);

  void VisitPreOrder(ControlFlowGraphVisitor *visitor);
  void VisitPostOrder(ControlFlowGraphVisitor *visitor);
  void GarbageCollect(void);
};

}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_CONTEXT_H_
