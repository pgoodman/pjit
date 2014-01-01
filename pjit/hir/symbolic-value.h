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

struct StructureFieldInfo;

namespace mir {
class Symbol;
}  // namespace mir

namespace hir {


// Represents a value where the type of the value is both known at runtime by
// the `Symbol`, and at compile time by the `T` type parameter to the
// class template.
template <typename T>
class SymbolicValue {
 public:
  inline SymbolicValue(void)
      : symbol(nullptr) {}

  inline explicit SymbolicValue(const mir::Symbol *value)
      : symbol(value) {}

  inline void AssignSymbol(const mir::Symbol *value) {
    symbol = value;
  }

  inline const mir::Symbol *GetSymbol(void) const {
    return symbol;
  }

 private:
  const mir::Symbol *symbol;
};


enum class SymbolicValueReferenceKind {
  MEMORY,
  FIELD,
  INDEX
};


// Represents a reference to a value of a memory cell, where at the time of
// creating the reference, we are not sure if the reference will be an r-value
// or an l-value.
template <typename T>
class SymbolicValueReference {
 public:
  const mir::Symbol * const symbol;
  const SymbolicValueReferenceKind kind;
  const union MetaInfo {
    int array_index;
    const StructureFieldInfo *field_info;

    MetaInfo(int index)
        : array_index(index) {}

    MetaInfo(const StructureFieldInfo *info)
        : field_info(info) {}
  } meta;

  explicit SymbolicValueReference(const mir::Symbol *value)
      : symbol(value),
        kind(SymbolicValueReferenceKind::MEMORY),
        meta(0) {}

  SymbolicValueReference(const mir::Symbol *value, unsigned array_index)
      : symbol(value),
        kind(SymbolicValueReferenceKind::INDEX),
        meta(array_index) {}

  SymbolicValueReference(const mir::Symbol *value, const StructureFieldInfo *field)
      : symbol(value),
        kind(SymbolicValueReferenceKind::INDEX),
        meta(field) {}

 private:
  SymbolicValueReference(void) = delete;
};

}  // namespace hir
}  // namespace pjit

#endif  // PJIT_HIR_SYMBOLIC_VALUE_H_
