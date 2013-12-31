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
inline const mir::Symbol *GetLValue(const SymbolicValue<T> value) {
  return value.GetSymbol();
}


template <typename T>
inline const mir::Symbol *GetRValue(mir::Context &,
                               const SymbolicValue<T> value) {
  return value.GetSymbol();
}


template <typename T>
inline const mir::Symbol *GetLValue(SymbolicValueReference<T> value) {
  return value.GetSymbol();
}


template <typename T>
inline const mir::Symbol *GetRValue(mir::Context &context,
                                    const SymbolicValueReference<T> value) {
  const mir::Symbol *dest(context.MakeSymbol(GetTypeInfoForType<T>()));
  const mir::Symbol *source(value.GetSymbol());
  context.EmitInstruction(mir::Operation::OP_LOAD_MEMORY, {dest, source});
  return dest;
}


template <typename T>
inline const mir::Symbol *GetLValue(SymbolicVariable<T> &var) {
  return var.GetSymbol();
}


template <typename T>
inline const mir::Symbol *GetRValue(mir::Context &, const SymbolicVariable<T> &var) {
  return var.GetSymbol();
}


// Getting an L-value for a constant is not well-defined. This should not be
// calls.
template <
  typename T,
  typename EnableIf<StaticTypeInfoFactory<T>::IS_DEFINED, int>::Type=0
>
inline const mir::Symbol *GetLValue(T &) {
  // TODO(pag): Assert that this isn't invoked.
  return nullptr;
}


// Get a symbol for a constant. We only allow constants for known types to be
// loaded as immediates. In practice, only integral, floating point, and pointer
// types should be used.
template <
  typename T,
  typename EnableIf<StaticTypeInfoFactory<T>::IS_DEFINED, int>::Type=0
>
inline const mir::Symbol *GetRValue(mir::Context &context, T val) {
  return context.MakeSymbol(val);
}


#define PJIT_HIR_DECLARED_TYPE(...) __VA_ARGS__


#define PJIT_HIR_DECLARE(context, type, name) \
  pjit::hir::SymbolicVariable<PJIT_HIR_DECLARED_TYPE type> name( \
    context.MakeSymbol( \
      pjit::GetTypeInfoForType<PJIT_HIR_DECLARED_TYPE type>(), \
      PJIT_TO_STRING(name)));


#define PJIT_DECLARE_BINARY_OPERATOR(name, op) \
  template <typename L, typename R> \
  SymbolicValue< \
    typename RemoveReference< \
      decltype( \
        typename TypeOfSymbolicValue<L>::Type() op \
        typename TypeOfSymbolicValue<R>::Type() \
      ) \
    >::Type \
  > \
  inline name (mir::Context &context, L &&left, R &&right) { \
    typedef typename TypeOfSymbolicValue<L>::Type LeftType; \
    typedef typename TypeOfSymbolicValue<R>::Type RightType; \
    typedef decltype(LeftType() op RightType()) RefOutputType; \
    typedef typename RemoveReference<RefOutputType>::Type OutputType; \
    const TypeInfo *output_type(GetTypeInfoForType<OutputType>()); \
    const mir::Symbol *left_conv(GetRValue(context, left)); \
    const mir::Symbol *right_conv(GetRValue(context, right)); \
    if (!TypesAreEqual<bool, OutputType>::RESULT) { \
      left_conv = context.EmitConvertType(output_type, left_conv); \
      right_conv = context.EmitConvertType(output_type, right_conv); \
    } \
    const mir::Symbol *output_value(context.MakeSymbol(output_type)); \
    context.EmitInstruction( \
        mir::Operation::PJIT_CAT(OP_, name), \
        {output_value, left_conv, right_conv}); \
    return SymbolicValue<OutputType>(output_value); \
  }


#define PJIT_DECLARE_UNARY_OPERATOR(name, op) \
  template <typename R> \
  SymbolicValue< \
    typename RemoveReference< \
      decltype(op typename TypeOfSymbolicValue<R>::Type()) \
    >::Type \
  > \
  inline name (mir::Context &context, SymbolicValue<R> &&right) { \
    typedef typename TypeOfSymbolicValue<R>::Type RightType; \
    typedef decltype(op RightType()) RefOutputType; \
    typedef typename RemoveReference<RefOutputType>::Type OutputType; \
    const TypeInfo *output_type(GetTypeInfoForType<OutputType>()); \
    const mir::Symbol *right_conv(GetRValue(context, right)); \
    if (!TypesAreEqual<bool, OutputType>::RESULT) { \
      context.EmitConvertType(output_type, right_conv); \
    } \
    const mir::Symbol *output_value(context.MakeSymbol(output_type)); \
    context.EmitInstruction( \
        mir::Operation::PJIT_CAT(OP_, name), {output_value, right_conv}); \
    return SymbolicValue<OutputType>(output_value); \
  }


#include "pjit/mir/operator.h"
#undef PJIT_DECLARE_BINARY_OPERATOR
#undef PJIT_DECLARE_UNARY_OPERATOR


// Type trait for detecting whether or not a type is a symbolic value
// reference. Distinguishing between symbolic value refereces, variables, and
// plain-old values is important when performing a memory store.
template <typename T>
struct IsSymbolicValueReference {
  enum {
    RESULT = false
  };
};


template <typename T>
struct IsSymbolicValueReference<SymbolicValueReference<T>> {
  enum {
    RESULT = true
  };
};


// Emit an instruction that loads a value from memory. If we're tring to load
// a symbolic value reference, then that means the input was the result of
// another `LOAD_MEMORY` call, and so we rightly resolve its r-value (by
// internally emitting a `OP_LOAD_MEMORY` instruction. Otherwise, we just get
// the symbol's l-value.
template <typename T>
SymbolicValueReference<
  typename RemoveReference<
    decltype(*typename TypeOfSymbolicValue<T>::Type())
  >::Type
>
inline LOAD_MEMORY(mir::Context &context, T &&right) {
  typedef typename TypeOfSymbolicValue<T>::Type RightType;
  typedef decltype(*RightType()) RefOutputType;
  typedef typename RemoveReference<RefOutputType>::Type OutputType;
  if (IsSymbolicValueReference<T>::RESULT) {
    return SymbolicValueReference<OutputType>(GetRValue(context, right));
  } else {
    return SymbolicValueReference<OutputType>(GetLValue(right));
  }
}


// Assign one value to another. If the left-hand side of the assignment is a
// dereferenced address (symbolic value reference), then we emit a
// `OP_STORE_MEMORY` instruction, otherwise we use a plain-old `OP_ASSIGN`
// instruction.
template <typename L, typename R>
SymbolicValue<typename TypeOfSymbolicValue<L>::Type>
inline ASSIGN(mir::Context &context, L &&left, R &&right) {
  typedef typename TypeOfSymbolicValue<L>::Type RefOutputType;
  typedef typename RemoveReference<RefOutputType>::Type OutputType;

  const TypeInfo *output_type_info(GetTypeInfoForType<OutputType>());
  const mir::Symbol *right_conv(context.EmitConvertType(
      output_type_info, GetRValue(context, right)));

  // Storing through a memory reference.
  if (IsSymbolicValueReference<L>::RESULT) {
    context.EmitInstruction(
        mir::Operation::OP_STORE_MEMORY, {GetLValue(left), right_conv});

  // Assigning the value of an abstract register.
  } else {
    context.EmitInstruction(
        mir::Operation::OP_ASSIGN, {GetLValue(left), right_conv});
  }

  return SymbolicValue<OutputType>(right_conv);
}


// Interacts with the MIR context to construct ELSE statements.
//
// Note: This assumes that the ELSE statement is being used correctly (i.e.
//       within an IF statement.
class ElseStatementBuilder {
 public:
  explicit ElseStatementBuilder(mir::Context &context);

 private:
  ElseStatementBuilder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ElseStatementBuilder);
};


// Interacts with the MIR context to construct IF statements.
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


#define PJIT_HIR_SYM_ID PJIT_CAT(pjit_hir_sym_, __LINE__)


#define PJIT_HIR_IF(context, cond) \
  { \
    pjit::hir::IfStatementBuilder PJIT_HIR_SYM_ID (context); \
    PJIT_HIR_SYM_ID.Condition(cond); \
    {


#define PJIT_HIR_ELSE(context) \
    } \
    pjit::hir::ElseStatementBuilder PJIT_HIR_SYM_ID(context); \
    PJIT_UNUSED(PJIT_HIR_SYM_ID); \
    {


#define PJIT_HIR_END_IF \
    } \
  }


// Interacts with the MIR context to construct CASE statements.
//
// Note: This assumes that the ELSE statement is being used correctly (i.e.
//       within an IF statement.
class CaseStatementBuilder {
 public:
  explicit CaseStatementBuilder(mir::Context &context,
                                const mir::Symbol *symbol);
  ~CaseStatementBuilder(void);

 private:
  mir::Context * const context;

  CaseStatementBuilder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(CaseStatementBuilder);
};


// Interacts with the MIR context to construct SWITCH statements.
class SwitchStatementBuilder {
 public:
  explicit SwitchStatementBuilder(mir::Context &context_);
  ~SwitchStatementBuilder(void);

  // The type is already a boolean, no conversion is necessary.
  template <typename T>
  void Condition(T &&val) {
    mbr->conditional_value = GetRValue(*context, val);

    // Ensure that emitting instructions without a CASE statement results in
    // a fault.
    context->current = nullptr;
  }

 private:
  friend class CaseStatementBuilder;

  mir::Context *context;
  mir::MultiWayBranchControlFlowGraph *mbr;
  mir::SequentialControlFlowGraph *successor;

  SwitchStatementBuilder *saved_mbr_builder;

  SwitchStatementBuilder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(SwitchStatementBuilder);
};


#define PJIT_HIR_SWITCH(context, cond) \
  { \
    pjit::hir::SwitchStatementBuilder PJIT_HIR_SYM_ID (context); \
    PJIT_HIR_SYM_ID.Condition(cond); \


#define PJIT_HIR_DEFAULT(context) \
  { \
    pjit::hir::CaseStatementBuilder PJIT_HIR_SYM_ID( \
        (context), nullptr); \
    PJIT_UNUSED(PJIT_HIR_SYM_ID); \
  }


#define PJIT_HIR_CASE(context, value) \
  { \
    pjit::hir::CaseStatementBuilder PJIT_HIR_SYM_ID( \
        (context), (context).MakeSymbol(value)); \
    PJIT_UNUSED(PJIT_HIR_SYM_ID); \
    {

#define PJIT_HIR_END_CASE \
    } \
  }


#define PJIT_HIR_END_SWITCH \
  }


class LoopStatementBuilder {
 public:
  explicit LoopStatementBuilder(mir::Context &context_);
  ~LoopStatementBuilder(void);

  // The type is already a boolean, no conversion is necessary.
  template <typename T>
  void Condition(T &&val) {
    loop->conditional_value = GetRValue(*context, val);

    // Ensure that emitting instructions without a CASE statement results in
    // a fault.
    context->current = &(loop->update);
  }

  inline void Init(void) {
    context->current = &(loop->condition);
  }

  inline void Update(void) {
    context->current = &(loop->body);
  }

 private:
  mir::Context *context;
  mir::LoopControlFlowGraph *loop;
  mir::SequentialControlFlowGraph *successor;
};


#define PJIT_HIR_FOR(context, init, test, next) \
  { \
    pjit::hir::LoopStatementBuilder PJIT_HIR_SYM_ID(context); \
    PJIT_UNPACK init ; \
    PJIT_HIR_SYM_ID.Init(); \
    PJIT_HIR_SYM_ID.Condition(test); \
    { PJIT_UNPACK next ; } \
    PJIT_HIR_SYM_ID.Update(); \
    { \


#define PJIT_HIR_END_FOR \
    } \
  }


}  // namespace hir
}  // namespace pjit


#endif  // PJIT_HIR_HIR_TO_MIR_H_
