/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * type-info.cc
 *
 *  Created on: 2013-12-19
 *      Author: Peter Goodman
 */


#include "pjit/base/type-info.h"


#define PJIT_DEFINE_BASIC_TYPE_INFO(type, kind) \
  PJIT_DEFINE_CUSTOM_TYPE_INFO( \
      GenericTypeInfo, \
      type, \
      sizeof(type), \
      PJIT_ALIGNMENT_OF(type), \
      kind, )


#define PJIT_DEFINE_INTEGRAL_TYPE_INFO(type, kind, is_signed, overflow) \
  PJIT_DEFINE_CUSTOM_TYPE_INFO( \
      IntegerTypeInfo, \
      type, \
      sizeof(type), \
      PJIT_ALIGNMENT_OF(type), \
      kind, \
      is_signed, \
      IntegerOverflowBehavior::overflow)


namespace pjit {


PJIT_DEFINE_CUSTOM_TYPE_INFO(
    GenericTypeInfo,
    void,
    0,
    0,
    TYPE_KIND_UNDEFINED, );  // NOLINT


PJIT_DEFINE_CUSTOM_TYPE_INFO(
    BooleanTypeInfo,
    bool,
    sizeof(bool),
    PJIT_ALIGNMENT_OF(bool),  // NOLINT
    TYPE_KIND_BOOLEAN, );  // NOLINT


PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    U8, TYPE_KIND_INTEGER, false, INTEGER_OVERFLOW_WRAPS);
PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    S8, TYPE_KIND_INTEGER, true, INTEGER_OVERFLOW_UNDEFINED);
PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    U16, TYPE_KIND_INTEGER, false, INTEGER_OVERFLOW_WRAPS);
PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    S16, TYPE_KIND_INTEGER, true, INTEGER_OVERFLOW_UNDEFINED);
PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    U32, TYPE_KIND_INTEGER, false, INTEGER_OVERFLOW_WRAPS);
PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    S32, TYPE_KIND_INTEGER, true, INTEGER_OVERFLOW_UNDEFINED);
PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    U64, TYPE_KIND_INTEGER, false, INTEGER_OVERFLOW_WRAPS);
PJIT_DEFINE_INTEGRAL_TYPE_INFO(
    S64, TYPE_KIND_INTEGER, true, INTEGER_OVERFLOW_UNDEFINED);


PJIT_DEFINE_BASIC_TYPE_INFO(F32, TYPE_KIND_FLOATING_POINT);
PJIT_DEFINE_BASIC_TYPE_INFO(F64, TYPE_KIND_FLOATING_POINT);

}  // namespace pjit

