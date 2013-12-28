/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * symbol.h
 *
 *  Created on: 2013-12-24
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_SYMBOL_H_
#define PJIT_BASE_SYMBOL_H_

#include "pjit/base/type-info.h"
#include "pjit/base/type-traits.h"

namespace pjit {

class Symbol {
 private:

  Symbol(void) = delete;
  Symbol(const Symbol &) = delete;
  Symbol(const Symbol &&) = delete;

  Symbol &operator=(const Symbol &) = delete;
  Symbol &operator=(const Symbol &&) = delete;

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
  const unsigned id;

  Symbol(const TypeInfo *type_, const char *name_, unsigned id_)
      : type(type_),
        id(id_) {
    value.name = name_;
  }

  Symbol(const TypeInfo *type_, void *pointer)
      : type(type_),
        id(0) {
    value.pointer = pointer;
  }

#define MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(type_name, field) \
  explicit Symbol(type_name val) \
      : type(GetTypeInfoForType<type_name>()), \
        id(0) { \
    value.field = val; \
  }

  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(U8, u8)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(S8, s8)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(U16, u16)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(S16, s16)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(U32, u32)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(S32, s32)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(U64, u64)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(S64, s64)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(F32, f32)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(F64, f64)
  MAKE_SIMPLE_SYMBOL_CONSTRUCTOR(decltype(nullptr), pointer)

  template <typename T>
  explicit Symbol(T *val)
      : type(GetTypeInfoForType<T *>()),
        id(0) {
    value.pointer = reinterpret_cast<void *>(
        const_cast<typename RemoveConst<T>::Type *>(val));
  }
};


}  // namespace pjit

#endif  // PJIT_BASE_SYMBOL_H_
