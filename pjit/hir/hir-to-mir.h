/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * hir-to-mir.h
 *
 *  Created on: 2013-12-21
 *      Author: Peter Goodman
 */

#ifndef PJIT_HIR_HIR_TO_MIR_H_
#define PJIT_HIR_HIR_TO_MIR_H_

#include "pjit/base/base.h"
#include "pjit/base/type-info.h"
#include "pjit/base/type-traits.h"

#include "pjit/hir/symbolic-value.h"
#include "pjit/hir/symbolic-variable.h"

#include "pjit/mir/instruction.h"
#include "pjit/mir/context.h"


namespace pjit {
namespace hir {


template <typename T>
struct TypeOfSymbolicValue {
  typedef T Type;
};


#define PJIT_DEFINE_TYPE_OF_VALUE_IMPL(tpl, prefix, suffix) \
  template <typename T> \
  struct TypeOfSymbolicValue<prefix tpl<T> suffix> { \
    typedef T Type; \
  };


#define PJIT_DEFINE_TYPE_OF_VALUE(tpl) \
  PJIT_DEFINE_TYPE_OF_VALUE_IMPL(tpl, , ) \
  PJIT_DEFINE_TYPE_OF_VALUE_IMPL(tpl, , &) \
  PJIT_DEFINE_TYPE_OF_VALUE_IMPL(tpl, , &&) \
  PJIT_DEFINE_TYPE_OF_VALUE_IMPL(tpl, const, ) \
  PJIT_DEFINE_TYPE_OF_VALUE_IMPL(tpl, const, &) \
  PJIT_DEFINE_TYPE_OF_VALUE_IMPL(tpl, const, &&)


PJIT_DEFINE_TYPE_OF_VALUE(SymbolicValue)
PJIT_DEFINE_TYPE_OF_VALUE(SymbolicValueReference)
PJIT_DEFINE_TYPE_OF_VALUE(SymbolicVariable)


#undef PJIT_DEFINE_TYPE_OF_VALUE_IMPL
#undef PJIT_DEFINE_TYPE_OF_VALUE


template <typename T>
inline const Symbol *GetLValue(const SymbolicValue<T> value) {
  return value.GetSymbol();
}


template <typename T>
inline const Symbol *GetRValue(mir::Context &,
                               const SymbolicValue<T> value) {
  return value.GetSymbol();
}


template <typename T>
inline const Symbol *GetLValue(SymbolicValueReference<T> value) {
  return value.GetSymbol();
}


template <typename T>
inline const Symbol *GetRValue(mir::Context &context,
                               const SymbolicValueReference<T> value) {
  const Symbol *dest(context.MakeSymbol(GetTypeInfoForType<T>()));
  const Symbol *source(value.GetSymbol());
  context.EmitInstruction(mir::Operation::kOpLoadMemory, {dest, source});
  return dest;
}


template <typename T>
inline const Symbol *GetLValue(SymbolicVariable<T> &var) {
  return var.GetSymbol();
}


template <typename T>
inline const Symbol *GetRValue(mir::Context &, const SymbolicVariable<T> &var) {
  return var.GetSymbol();
}


template <
  typename T,
  typename EnableIf<StaticTypeInfoFactory<T>::IS_DEFINED, int>::Type=0
>
inline const Symbol *GetLValue(T &) {
  return nullptr;
}


template <
  typename T,
  typename EnableIf<StaticTypeInfoFactory<T>::IS_DEFINED, int>::Type=0
>
inline const Symbol *GetRValue(mir::Context &context, T &val) {
  const TypeInfo *type(GetTypeInfoForType<T>());
  const Symbol *dest(context.MakeSymbol(type));
  context.EmitInstruction(
      mir::Operation::kOpLoadImmediate,
      {dest, context.MakeSymbol(val)});
  return dest;
}


#define PJIT_HIR_DECLARED_TYPE(...) __VA_ARGS__


#define PJIT_HIR_DECLARE(context, type, name) \
  pjit::hir::SymbolicVariable<PJIT_HIR_DECLARED_TYPE type> name( \
    context.MakeSymbol( \
      pjit::GetTypeInfoForType<PJIT_HIR_DECLARED_TYPE type>(), \
      #name));


#define PJIT_DECLARE_BINARY_OPERATOR(name, op) \
  template <typename L, typename R> \
  SymbolicValue< \
    typename RemoveReference< \
      decltype( \
        typename TypeOfSymbolicValue<L>::Type() op typename TypeOfSymbolicValue<R>::Type() \
      ) \
    >::Type \
  > \
  inline name (mir::Context &context, L &&left, R &&right) { \
    typedef typename TypeOfSymbolicValue<L>::Type LeftType; \
    typedef typename TypeOfSymbolicValue<R>::Type RightType; \
    typedef decltype(LeftType() op RightType()) RefOutputType; \
    typedef typename RemoveReference<RefOutputType>::Type OutputType; \
    const TypeInfo *output_type(GetTypeInfoForType<OutputType>()); \
    const Symbol *left_conv(GetRValue(context, left)); \
    const Symbol *right_conv(GetRValue(context, right)); \
    if (!TypesAreEqual<bool, OutputType>::RESULT) { \
      left_conv = context.EmitConvertType(output_type, left_conv); \
      right_conv = context.EmitConvertType(output_type, right_conv); \
    } \
    const Symbol *output_value(context.MakeSymbol(output_type)); \
    context.EmitInstruction( \
        mir::Operation::kOp ## name, {output_value, left_conv, right_conv}); \
    return SymbolicValue<OutputType>(output_value); \
  }


#define PJIT_DECLARE_UNARY_OPERATOR(name, op) \
  template <typename R> \
  SymbolicValue< \
    typename RemoveReference< \
      decltype( \
        op typename TypeOfSymbolicValue<R>::Type() \
      ) \
    >::Type \
  > \
  inline name (mir::Context &context, SymbolicValue<R> &&right) { \
    typedef typename TypeOfSymbolicValue<R>::Type RightType; \
    typedef decltype(op RightType()) RefOutputType; \
    typedef typename RemoveReference<RefOutputType>::Type OutputType; \
    const TypeInfo *output_type(GetTypeInfoForType<OutputType>()); \
    const Symbol *right_conv(GetRValue(context, right)); \
    if (!TypesAreEqual<bool, OutputType>::RESULT) { \
      context.EmitConvertType(output_type, right_conv); \
    } \
    const Symbol *output_value(context.MakeSymbol(output_type)); \
    context.EmitInstruction( \
        mir::Operation::kOp ## name, {output_value, right_conv}); \
    return SymbolicValue<OutputType>(output_value); \
  }


#include "pjit/mir/operator.h"
#undef PJIT_DECLARE_BINARY_OPERATOR
#undef PJIT_DECLARE_UNARY_OPERATOR


template <typename T>
inline bool IsSymbolicValueReference(const T &) {
  return false;
}


template <typename T>
inline bool IsSymbolicValueReference(const SymbolicValueReference<T> &) {
  return true;
}


template <typename T>
SymbolicValueReference<
  typename RemoveReference<
    decltype(*typename TypeOfSymbolicValue<T>::Type())
  >::Type
>
inline LoadMemory(mir::Context &context, T &&right) {
  typedef typename TypeOfSymbolicValue<T>::Type RightType;
  typedef decltype(*RightType()) RefOutputType;
  typedef typename RemoveReference<RefOutputType>::Type OutputType;
  if (IsSymbolicValueReference(right)) {
    return SymbolicValueReference<OutputType>(GetRValue(context, right));
  } else {
    return SymbolicValueReference<OutputType>(GetLValue(right));
  }
}


// Assign one value to another.
template <typename L, typename R>
SymbolicValue<typename TypeOfSymbolicValue<L>::Type>
inline Assign(mir::Context &context, L &&left, R &&right) {
  typedef typename TypeOfSymbolicValue<L>::Type RefOutputType;
  typedef typename RemoveReference<RefOutputType>::Type OutputType;
  const TypeInfo *output_type_info(GetTypeInfoForType<OutputType>());

  const Symbol *right_conv(context.EmitConvertType(
      output_type_info, GetRValue(context, right)));

  // Storing through a memory reference.
  if (IsSymbolicValueReference(left)) {
    context.EmitInstruction(
        mir::Operation::kOpStoreMemory, {GetLValue(left), right_conv});

  // Assigning the value of an abstract register.
  } else {
    context.EmitInstruction(
        mir::Operation::kOpAssign, {GetLValue(left), right_conv});
  }

  return SymbolicValue<OutputType>(right_conv);
}


class ElseStatementBuilder {
 public:
  explicit ElseStatementBuilder(mir::Context &context);

 private:
  ElseStatementBuilder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ElseStatementBuilder);
};


class IfStatementBuilder {
 public:
  explicit IfStatementBuilder(mir::Context &context_);
  ~IfStatementBuilder(void);

  // The type is already a boolean, no conversion is necessary.
  template <
    typename T,
    typename EnableIf<
      TypesAreEqual<bool, typename TypeOfSymbolicValue<T>::Type>::RESULT,
      int
    >::Type = 0
  >
  void Condition(T &&val) {
    conditional->conditional_value = GetRValue(*context, val);
    context->current = &(conditional->if_true);
  }

  // The type is not a boolean, some conversion might be necessary.
  template <
    typename T,
    typename EnableIf<
      TypesAreEqual<bool, typename TypeOfSymbolicValue<T>::Type>::RESULT,
      void,
      int
    >::Type = 0
  >
  void Condition(T &&val) {
    conditional->conditional_value = context->EmitConvertType(
        GetTypeInfoForType<bool>(), GetRValue(*context, val));
  }

 private:

  friend class ElseStatementBuilder;

  mir::Context *context;
  mir::ConditionalControlFlowGraph *conditional;
  mir::SequentialControlFlowGraph *successor;

  IfStatementBuilder *saved_if_builder;

  IfStatementBuilder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(IfStatementBuilder);
};


#define PJIT_HIR_IF_ID PJIT_CAT(pjit_hir_if_, __LINE__)


#define PJIT_HIR_IF(context, cond) \
    { \
      pjit::hir::IfStatementBuilder PJIT_HIR_IF_ID (context); \
      PJIT_HIR_IF_ID.Condition(cond); \
      { \


#define PJIT_HIR_ELSE(context) \
      } \
      pjit::hir::ElseStatementBuilder PJIT_HIR_IF_ID(context); \
      {


#define PJIT_HIR_END_IF \
      } \
    }


}  // namespace hir
}  // namespace pjit


#endif  // PJIT_HIR_HIR_TO_MIR_H_
