/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * unsafe-cast.h
 *
 *  Created on: 2013-12-27
 *    Author: Peter Goodman
 */

#ifndef PJIT_BASE_UNSAFE_CAST_H_
#define PJIT_BASE_UNSAFE_CAST_H_

#include "pjit/base/numeric-types.h"
#include "pjit/base/type-traits.h"

namespace pjit {

// Non-integral, non-pointer type to something else.
//
// Note: `__builtin_memcpy` is used instead of `memcpy`, mostly for the
//     sake of kernel code where it sometimes seems that the optimisation
//     of inlining a normal `memcpy` is not done.
template <
  typename ToT,
  typename FromT,
  typename EnableIf<
    IsPointer<FromT>::RESULT || IsInteger<FromT>::RESULT,
    void,
    int
  >::Type = 0
>
inline ToT UnsafeCast(const FromT v) {
  static_assert(sizeof(FromT) == sizeof(ToT),
    "Dangerous unsafe cast between two types of different sizes.");

  ToT dest;
  __builtin_memcpy(&dest, &v, sizeof(ToT));
  return dest;
}


// Pointer to integral type.
template <
  typename ToT,
  typename FromT,
  typename EnableIf<
    IsPointer<FromT>::RESULT && IsInteger<ToT>::RESULT,
    int
  >::Type = 0
>
inline ToT UnsafeCast(const FromT v) {
  return static_cast<ToT>(reinterpret_cast<UnsignedPointer>(v));
}


// Pointer to pointer type.
template <
  typename ToT,
  typename FromT,
  typename EnableIf<
    IsPointer<FromT>::RESULT && IsPointer<ToT>::RESULT,
    int
  >::Type = 0
>
inline ToT UnsafeCast(const FromT v) {
  return reinterpret_cast<ToT>(reinterpret_cast<UnsignedPointer>(v));
}


// Integral to pointer type.
template <
  typename ToT,
  typename FromT,
  typename EnableIf<
  IsInteger<FromT>::RESULT && IsPointer<ToT>::RESULT,
    int
  >::Type = 0
>
inline ToT UnsafeCast(const FromT v) {
  return reinterpret_cast<ToT>(static_cast<UnsignedPointer>(v));
}

}  // namespace pjit

#endif  // PJIT_BASE_UNSAFE_CAST_H_
