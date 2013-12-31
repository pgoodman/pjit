/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * symbolic-variable.h
 *
 *  Created on: 2013-12-19
 *      Author: Peter Goodman
 */

#ifndef PJIT_HIR_SYMBOLIC_VARIABLE_H_
#define PJIT_HIR_SYMBOLIC_VARIABLE_H_


namespace pjit {

namespace mir {
class Symbol;
}  // namespace mir

namespace hir {


// Defines a lexically scoped variable for use within a specific translation
// context. Unlike a SymbolicValue, a variable is treated as non-temporary.
template <typename T>
class SymbolicVariable {
 public:
  inline explicit SymbolicVariable(const mir::Symbol *symbol)
      : value(symbol) {}

  inline const mir::Symbol *GetSymbol(void) const {
    return value;
  }

 private:
  const mir::Symbol *value;

  SymbolicVariable(void) = delete;
  SymbolicVariable(const SymbolicVariable<T> &) = delete;
  SymbolicVariable(const SymbolicVariable<T> &&) = delete;

  PJIT_DISALLOW_ASSIGN(SymbolicVariable<T>);
};


} // namespace hir
} // namespace pjit

#endif  // PJIT_HIR_SYMBOLIC_VARIABLE_H_
