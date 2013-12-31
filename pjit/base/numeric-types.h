// Copyright 2013, Peter Goodman
// All rights reserved.
//
// Author: peter.goodman@gmail.com (Peter Goodman)
//
// Defines basic integral and floating point types.

#ifndef PJIT_BASE_NUMERIC_TYPES_H_
#define PJIT_BASE_NUMERIC_TYPES_H_


namespace pjit {


namespace {
  // Defines a simple integral type chooser, and a static size check on that
  // type.
  template <const unsigned kSize, typename T>
  struct CheckAndDefineType {
    static_assert(kSize == sizeof(T), "Unexpected size of integral type.");
    typedef T Type;
  };
} // namespace


typedef CheckAndDefineType<1, char>::Type S8;
typedef CheckAndDefineType<1, unsigned char>::Type U8;

typedef CheckAndDefineType<2, short>::Type S16;
typedef CheckAndDefineType<2, unsigned short>::Type U16;

typedef CheckAndDefineType<4, int>::Type S32;
typedef CheckAndDefineType<4, unsigned>::Type U32;

typedef CheckAndDefineType<8, long>::Type S64;
typedef CheckAndDefineType<8, unsigned long>::Type U64;

typedef CheckAndDefineType<4, float>::Type F32;
typedef CheckAndDefineType<8, double>::Type F64;

static_assert(
    sizeof(void *) == sizeof(U64),
    "numeric-types.h assumes 64-bit architecture, where `long` is a 64 bit "
    "int. This file should be generalized to support more architectures, given "
    "that your build is failing.");

typedef S64 SignedPointer;
typedef U64 UnsignedPointer;

typedef decltype(sizeof(SignedPointer)) UnsignedSize;

}  // namespace pjit

#endif  // PJIT_BASE_NUMERIC_TYPES_H_
