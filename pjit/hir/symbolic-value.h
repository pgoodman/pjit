/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * symbolic-value.h
 *
 *  Created on: 2013-12-20
 *      Author: Peter Goodman
 */

#ifndef PJIT_HIR_SYMBOLIC_VALUE_H_
#define PJIT_HIR_SYMBOLIC_VALUE_H_


namespace pjit {

class Symbol;

namespace hir {


// Represents a value where the type of the value is both known at runtime by
// the `Symbol`, and at compile time by the `T` type parameter to the
// class template.
template <typename T>
class SymbolicValue {
 private:

  const Symbol *symbol;

 public:

  inline SymbolicValue(void)
      : symbol(nullptr) {}

  inline explicit SymbolicValue(const Symbol *value)
      : symbol(value) {}

  inline void AssignSymbol(const Symbol *value) {
    symbol = value;
  }

  inline const Symbol *GetSymbol(void) const {
    return symbol;
  }
};


// Represents a reference to a value of a memory cell, where at the time of
// creating the reference, we are not sure if the reference will be an r-value
// or an l-value.
template <typename T>
class SymbolicValueReference {
 private:

  const Symbol *symbol;

  SymbolicValueReference(void) = delete;

 public:

  inline explicit SymbolicValueReference(const Symbol *value)
      : symbol(value) {}

  inline const Symbol *GetSymbol(void) const {
    return symbol;
  }
};

}  // namespace hir
}  // namespace pjit

#endif  // PJIT_HIR_SYMBOLIC_VALUE_H_
