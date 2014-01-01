/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * symbol.h
 *
 *  Created on: 2013-12-24
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_SYMBOL_H_
#define PJIT_MIR_SYMBOL_H_

#include "pjit/base/base.h"
#include "pjit/base/type-info.h"
#include "pjit/base/type-traits.h"
#include "pjit/base/visitor.h"

namespace pjit {
namespace mir {


enum SymbolBehavior : unsigned {
  BehaviorLocal             = (1 << 0),
  BehaviorGlobal            = (1 << 1),
  BehaviorPersistent        = (1 << 2),
  BehaviorProgramCounter    = (1 << 3) | BehaviorPersistent | BehaviorLocal,

  BehaviorFieldAccess       = (1 << 20)
};


// Represents a symbol (e.g. an immediate constant, or a variable/register/
// memory location) in the MIR.
class Symbol {
 public:
  const TypeInfo * const type;

  // The value of a symbol (its name if `id > 0`, or a pointer to some constant
  // in memory. The pointer value of `value` is always non-NULL. In the case of
  // a named symbol, `name` will always point to the empty string or a valid
  // name.
  union {
    const char *name;
    U8 u8;
    S8 s8;
    U16 u16;
    S16 s16;
    U32 u32;
    S32 s32;
    U64 u64;
    S64 s64;
    F32 f32;
    F64 f64;
    void *pointer;
  } value;

  // Note: Symbols with `id` zero are constants.
  unsigned id;
  SymbolBehavior behavior;

  Symbol(const TypeInfo *type_, const char *name_, unsigned id_);
  Symbol(const TypeInfo *type_, void *pointer);

#define PJIT_DECLARE_SYMBOL_CONSTRUCTOR(type_name, field) \
  explicit Symbol(type_name val);
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(U8, u8)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(S8, s8)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(U16, u16)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(S16, s16)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(U32, u32)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(S32, s32)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(U64, u64)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(S64, s64)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(F32, f32)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(F64, f64)
  PJIT_DECLARE_SYMBOL_CONSTRUCTOR(decltype(nullptr), pointer)
#undef PJIT_DECLARE_SYMBOL_CONSTRUCTOR

  // Makes a constant symbol for a pointer type.
  template <typename T>
  explicit Symbol(T *val)
      : type(GetTypeInfoForType<T *>()),
        id(0),
        behavior(SymbolBehavior::BehaviorLocal) {
    value.pointer = reinterpret_cast<void *>(
        const_cast<typename RemoveConst<T>::Type *>(val));
  }

 private:
  Symbol(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(Symbol);
};


class SymbolVisitor {
 public:
  SymbolVisitor(void) = default;
  virtual ~SymbolVisitor(void) = default;
  virtual void Visit(Symbol *) = 0;
};


}  // namespace mir


template <>
struct VisitorFor<mir::Symbol> {
  typedef mir::SymbolVisitor Type;
};

}  // namespace pjit

#endif  // PJIT_MIR_SYMBOL_H_
