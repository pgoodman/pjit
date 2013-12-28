/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * type-info.h
 *
 *  Created on: 2013-12-19
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_TYPE_INFO_H_
#define PJIT_BASE_TYPE_INFO_H_

#include "pjit/base/base.h"
#include "pjit/base/compiler.h"
#include "pjit/base/numeric-types.h"

namespace pjit {


// Defines the different C-like type kinds that are understood by the high-level
// intermediate representation.
enum class TypeKind {
  TYPE_KIND_UNDEFINED,
  TYPE_KIND_POINTER,
  TYPE_KIND_FUNCTION,
  TYPE_KIND_INTEGER,
  TYPE_KIND_BOOLEAN,
  TYPE_KIND_FLOATING_POINT,
  TYPE_KIND_STRUCTURE,
  TYPE_KIND_UNION
};


struct TypeInfo;


// Describes an individual field within a C-like structure or union. The
// description understands the field location at the bit granularity, rather
// than at the byte granularity, so as to handle bitfields.
struct StructureFieldInfo {
  const TypeInfo * const type;
  const unsigned offset_in_bits;
  const unsigned size_in_bits;
  const char * const name;
};


// Defines different types of integer overflow.
enum class IntegerOverflowBehavior {
  INTEGER_OVERFLOW_WRAPS,
  INTEGER_OVERFLOW_UNDEFINED
};


// Describes an individual C-like type.
struct TypeInfo {
  const unsigned size_in_bytes;
  const unsigned alignment;
  const TypeKind kind;
  const char *name;
};


struct GenericTypeInfo {
  const TypeInfo info;
};


struct PointerTypeInfo {
  TypeInfo info;
  const TypeInfo *pointed_to_type;
};


struct FunctionTypeInfo {
  TypeInfo info;
  const TypeInfo *return_type;
  const TypeInfo *argument_types;
  unsigned num_arguments;
};


struct IntegerTypeInfo {
  TypeInfo info;
  bool is_signed;
  IntegerOverflowBehavior overflow_behavior;
};


struct BooleanTypeInfo {
  TypeInfo info;
};


struct StructureTypeInfo {
  TypeInfo info;
  const StructureFieldInfo * fields;
  unsigned num_fields;
};


// Factory for creating TypeInfo structures at compile time.
template <typename T>
struct StaticTypeInfoFactory {
  enum {
    IS_DEFINED = false
  };
};



// Declares the static type information for a statically known type. Declaration
// is separate from definition to support recursive type references.
#define PJIT_DECLARE_TYPE_INFO(cls, type) \
  template <> \
  struct StaticTypeInfoFactory<type> { \
    enum { \
      IS_DEFINED = true \
    }; \
    static const cls kTypeInfo; \
  }


// Defines the static type information for a statically known type. Definition
// is separate from
#define PJIT_DEFINE_CUSTOM_TYPE_INFO(cls, type, size, align, kind, ...) \
  const cls StaticTypeInfoFactory<type>::kTypeInfo = { \
    {size, align, TypeKind::kind, PJIT_TO_STRING(type)}, \
    ##__VA_ARGS__ \
  }


// Forward declare type info for the various built-in types.
PJIT_DECLARE_TYPE_INFO(GenericTypeInfo, void);
PJIT_DECLARE_TYPE_INFO(BooleanTypeInfo, bool);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, U8);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, S8);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, U16);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, S16);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, U32);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, S32);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, U64);
PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, S64);
PJIT_DECLARE_TYPE_INFO(GenericTypeInfo, F32);
PJIT_DECLARE_TYPE_INFO(GenericTypeInfo, F64);


// Declare the type info for all unqualified pointer types.
template <typename T>
struct StaticTypeInfoFactory<T *> {
  enum {
    IS_DEFINED = true
  };
  static const PointerTypeInfo kTypeInfo;
};


// Recursively make the type info for a pointer type, based on the pointed-to
// type.
template <typename T>
const PointerTypeInfo StaticTypeInfoFactory<T *>::kTypeInfo = {
  {sizeof(T *), PJIT_ALIGNMENT_OF(T *), TypeKind::TYPE_KIND_POINTER, "*"},
  &(StaticTypeInfoFactory<T>::kTypeInfo.info)
};


#define PJIT_DECLARE_TYPE_INFO_ASSOCIATION(prefix, suffix) \
  template <typename T> \
  struct StaticTypeInfoFactory<prefix T suffix> \
      : public StaticTypeInfoFactory<T> \
  { \
    enum { \
      IS_DEFINED = true \
    }; \
  }


PJIT_DECLARE_TYPE_INFO_ASSOCIATION(const, *);
PJIT_DECLARE_TYPE_INFO_ASSOCIATION(volatile, *);
PJIT_DECLARE_TYPE_INFO_ASSOCIATION(const volatile, *);

PJIT_DECLARE_TYPE_INFO_ASSOCIATION(const, );
PJIT_DECLARE_TYPE_INFO_ASSOCIATION(volatile, );
PJIT_DECLARE_TYPE_INFO_ASSOCIATION(const volatile, );

PJIT_DECLARE_TYPE_INFO_ASSOCIATION(, &);
PJIT_DECLARE_TYPE_INFO_ASSOCIATION(, &&);

#undef PJIT_DECLARE_TYPE_INFO_ASSOCIATION

template <>
struct StaticTypeInfoFactory<decltype(nullptr)>
    : public StaticTypeInfoFactory<void *> {};

/*
// Recursively make the type info for a pointer type.
template <typename ReturnT, typename... ArgsT>
const TypeInfo StaticTypeInfoFactory<ReturnT (*)(ArgsT...)>::kTypeInfo = {
  sizeof(ReturnT (*)(ArgsT...)),
  PJIT_ALIGNMENT_OF(ReturnT (*)(ArgsT...)),
  TYPE_KIND_FUNCTION,
  {
      &(StaticTypeInfoFactory<ReturnT>::kTypeInfo),

      sizeof...(ArgsT)
  }
};
*/


// Returns a pointer to a TypeInfo structure associated with the type of the
// argument.
template <typename T>
inline const TypeInfo *GetTypeInfoForSymbolicValue(T &) {
  return &(StaticTypeInfoFactory<T>::kTypeInfo.info);
}


// Returns a pointer to a TypeInfo structure for the
template <typename T>
inline const TypeInfo *GetTypeInfoForType(void) {
  return &(StaticTypeInfoFactory<T>::kTypeInfo.info);
}

}  // namespace pjit

#endif  // PJIT_BASE_TYPE_INFO_H_
