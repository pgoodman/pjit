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
#include "pjit/base/unsafe-cast.h"
#include "pjit/base/libc.h"

namespace pjit {


// Defines the different C-like type kinds that are understood by the high-level
// intermediate representation.
enum class TypeKind {
  TYPE_KIND_UNDEFINED,
  TYPE_KIND_POINTER,
  TYPE_KIND_ARRAY,
  TYPE_KIND_FUNCTION,
  TYPE_KIND_INTEGER,
  TYPE_KIND_BOOLEAN,
  TYPE_KIND_FLOATING_POINT,
  TYPE_KIND_STRUCTURE,
  TYPE_KIND_UNION
};


struct TypeInfo;


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


struct ArrayTypeInfo {
  TypeInfo info;
  const TypeInfo *pointed_to_type;
  unsigned num_elements;
};


struct FunctionTypeInfo {
  TypeInfo info;
  const TypeInfo *return_type;
  const TypeInfo **argument_types;
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


// Describes an individual field within a C-like structure or union. The
// description understands the field location at the bit granularity, rather
// than at the byte granularity, so as to handle bitfields.
struct StructureFieldInfo {
  const TypeInfo * const type;
  enum {
    FIELD_NORMAL,
    FIELD_ARRAY,
    FIELD_BITFIELD
  } kind;
  union StructureFieldMetaData {
    unsigned array_length;
    struct {
      unsigned offset_in_bits;
      unsigned size_in_bits;
    } bit_field;
  } meta;
  const char * const name;
  void (*copy_field)(void *to, void *from);
};


struct StructureTypeInfo {
  TypeInfo info;
  const StructureFieldInfo *fields;
  unsigned num_fields;

  const StructureFieldInfo *GetFieldInfoForName(const char *name) const;
};


#define PJIT_DECLARE_STRUCTURE_TYPE_INFO(type_name) \
  PJIT_DECLARE_TYPE_INFO(StructureTypeInfo, type_name)


#define PJIT_DECLARE_UNION_TYPE_INFO(type_name) \
  PJIT_DECLARE_TYPE_INFO(StructureTypeInfo, type_name)


#define PJIT_DEFINE_STRUCTURE_TYPE_INFO_IMPL(type_name, kind, ...) \
  namespace PJIT_CAT(STRUCT_TYPE_INFO_, type_name) { \
    typedef type_name StructureTypeName; \
    static StructureFieldInfo FIELDS[] = { \
      __VA_ARGS__ \
    }; \
  } \
  const StructureTypeInfo StaticTypeInfoFactory<type_name>::kTypeInfo = { \
    { sizeof(type_name), \
      PJIT_ALIGNMENT_OF(type_name), \
      TypeKind::kind, \
      PJIT_TO_STRING(type_name) }, \
    &(PJIT_CAT(STRUCT_TYPE_INFO_, type_name)::FIELDS[0]), \
    (sizeof(PJIT_CAT(STRUCT_TYPE_INFO_, type_name)::FIELDS) / \
     sizeof(StructureFieldInfo)) \
  }


#define PJIT_SIMPLE_FIELD_COPY_FUNC(field_name) \
  [](void *to_, void *from_) { \
    StructureTypeName *to(UnsafeCast<StructureTypeName *>(to_)); \
    StructureTypeName *from(UnsafeCast<StructureTypeName *>(from_)); \
    to->field_name = from->field_name; \
  }


#define PJIT_ARRAY_FIELD_COPY_FUNC(field_name) \
  [](void *to_, void *from_) { \
    StructureTypeName *to(UnsafeCast<StructureTypeName *>(to_)); \
    StructureTypeName *from(UnsafeCast<StructureTypeName *>(from_)); \
    memcpy( \
        &(to->field_name[0]), \
        &(from->field_name[0]), \
        sizeof(to->field_name)); \
  }


#define PJIT_DEFINE_FIELD_IMPL(field_type, name, kind, num, copy_func) \
  { &(StaticTypeInfoFactory<PJIT_UNPACK field_type>::kTypeInfo.info), \
    StructureFieldInfo::kind, \
    {num}, \
    PJIT_TO_STRING(name), \
    copy_func(name) }


#define PJIT_DEFINE_FIELD(field_type, name) \
  PJIT_DEFINE_FIELD_IMPL( \
      field_type, \
      name, \
      FIELD_NORMAL, \
      1, \
      PJIT_SIMPLE_FIELD_COPY_FUNC)


#define PJIT_DEFINE_ARRAY_FIELD(base_type, array_len, name) \
  PJIT_DEFINE_FIELD_IMPL( \
      (PJIT_UNPACK base_type[array_len]), \
      name, \
      FIELD_ARRAY, \
      array_len, \
      PJIT_ARRAY_FIELD_COPY_FUNC)


#define PJIT_DEFINE_STRUCTURE_TYPE_INFO(type_name, ...) \
  PJIT_DEFINE_STRUCTURE_TYPE_INFO_IMPL(type_name, TYPE_KIND_STRUCTURE, __VA_ARGS__)


#define PJIT_DEFINE_UNION_TYPE_INFO(type_name, ...) \
  PJIT_DEFINE_STRUCTURE_TYPE_INFO_IMPL(type_name, TYPE_KIND_UNION, __VA_ARGS__)


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


/// Declares the static type information for an enumeration type.
#define PJIT_DECLARE_ENUM_TYPE_INFO(type) \
  PJIT_DECLARE_TYPE_INFO(IntegerTypeInfo, type)


/// Defined the static type information for an enumeration type.
#define PJIT_DEFINE_ENUM_TYPE_INFO(type) \
  PJIT_DEFINE_CUSTOM_TYPE_INFO( \
      IntegerTypeInfo, \
      type, \
      sizeof(type), \
      PJIT_ALIGNMENT_OF(type), \
      TYPE_KIND_INTEGER, \
      false, \
      IntegerOverflowBehavior::INTEGER_OVERFLOW_UNDEFINED)


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
template <typename T, unsigned kLen>
struct StaticTypeInfoFactory<T[kLen]> {
  enum {
    IS_DEFINED = true
  };
  static const ArrayTypeInfo kTypeInfo;
};


// Recursively make the type info for a pointer type, based on the pointed-to
// type.
template <typename T, unsigned kLen>
const ArrayTypeInfo StaticTypeInfoFactory<T[kLen]>::kTypeInfo = {
  { sizeof(T[kLen]),
    PJIT_ALIGNMENT_OF(T[kLen]),
    TypeKind::TYPE_KIND_ARRAY,
    "[]" },
  &(StaticTypeInfoFactory<T>::kTypeInfo.info),
  kLen
};


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


#if 0
namespace {


template <typename... Args>
struct FunctionArgumentUnroller;

template <typename Arg, typename... Args>
struct FunctionArgumentUnroller<Arg, Args...>
    : public FunctionArgumentUnroller<Arg> {
  const FunctionArgumentUnroller<Args...> kArgsRest;
};

template <typename Arg>
struct FunctionArgumentUnroller<Arg> {
  const TypeInfo * const kArgType =
      &(StaticTypeInfoFactory<Arg>::kTypeInfo.info);
};


}  // namespace


template <typename ReturnT, typename... ArgsT>
struct StaticTypeInfoFactory<ReturnT (*)(ArgsT...)> {
  enum {
    IS_DEFINED = true
  };
  static const FunctionArgumentUnroller<ArgsT...> kArgTypes;
  static const FunctionTypeInfo kTypeInfo;
};


// Recursively make the type info for a function pointer type.
template <typename ReturnT, typename... ArgsT>
const TypeInfo StaticTypeInfoFactory<ReturnT (*)(ArgsT...)>::kTypeInfo = {
  { sizeof(ReturnT (*)(ArgsT...)),
    PJIT_ALIGNMENT_OF(ReturnT (*)(ArgsT...)),
    TypeKind::TYPE_KIND_FUNCTION,
    "*"},
  {
      &(StaticTypeInfoFactory<ReturnT>::kTypeInfo.info),
      &(kArgTypes.kArgType),
      sizeof...(ArgsT)
  }
};
#endif


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
