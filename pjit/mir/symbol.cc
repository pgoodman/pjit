/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * symbol.cc
 *
 *  Created on: 2013-12-29
 *      Author: Peter Goodman
 */

#include "pjit/mir/symbol.h"

namespace pjit {
namespace mir {

Symbol::Symbol(const TypeInfo *type_, const char *name_, unsigned id_)
    : type(type_),
      id(id_) {
  value.name = name_;
}


Symbol::Symbol(const TypeInfo *type_, void *pointer)
    : type(type_),
      id(0) {
  value.pointer = pointer;
}


#define PJIT_DEFINE_SYMBOL_CONSTRUCTOR(type_name, field) \
  Symbol::Symbol(type_name val) \
      : type(GetTypeInfoForType<type_name>()), \
        id(0) { \
    value.field = val; \
  }

PJIT_DEFINE_SYMBOL_CONSTRUCTOR(U8, u8)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(S8, s8)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(U16, u16)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(S16, s16)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(U32, u32)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(S32, s32)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(U64, u64)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(S64, s64)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(F32, f32)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(F64, f64)
PJIT_DEFINE_SYMBOL_CONSTRUCTOR(decltype(nullptr), pointer)

#undef PJIT_DEFINE_SYMBOL_CONSTRUCTOR


}  // namespace mir
}  // namespace pjit
