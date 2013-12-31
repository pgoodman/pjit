/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * type-traits.h
 *
 *  Created on: 2013-12-21
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_TYPE_TRAITS_H_
#define PJIT_BASE_TYPE_TRAITS_H_


namespace pjit {


template <typename T>
struct RemoveReference {
  typedef T Type;
};


template <typename T>
struct RemoveReference<T &> {
  typedef T Type;
};


template <typename T>
struct RemoveReference<T &&> {
  typedef T Type;
};


template <const bool Condition, typename TrueType, typename FalseType=void>
struct EnableIf;


template <typename TrueType, typename FalseType>
struct EnableIf<true, TrueType, FalseType> {
  typedef TrueType Type;
};


template <typename TrueType, typename FalseType>
struct EnableIf<false, TrueType, FalseType> {
  typedef FalseType Type;
};


template <typename A, typename B>
struct TypesAreEqual {
  enum {
    RESULT = false
  };
};


template <typename A>
struct TypesAreEqual<A, A> {
  enum {
    RESULT = true
  };
};


template <typename A>
struct IsPointer {
  enum {
    RESULT = false
  };
};


template <typename A>
struct IsPointer<A *> {
  enum {
    RESULT = true
  };
};


template <typename A>
struct IsPointer<A &> {
  enum {
    RESULT = IsPointer<A>::RESULT
  };
};


template <typename A>
struct IsPointer<A &&> {
  enum {
    RESULT = IsPointer<A>::RESULT
  };
};


template <typename A>
struct IsInteger {
  enum {
    RESULT = false
  };
};


template <typename A>
struct IsInteger<A &> {
  enum {
    RESULT = IsInteger<A>::RESULT
  };
};


template <typename A>
struct IsInteger<A &&> {
  enum {
    RESULT = IsInteger<A>::RESULT
  };
};


#define PJIT_DEFINE_IS_INTEGRAL(type) \
  template <> \
  struct IsInteger<type> { \
    enum { \
      RESULT = true \
    }; \
  }
PJIT_DEFINE_IS_INTEGRAL(U8);
PJIT_DEFINE_IS_INTEGRAL(S8);
PJIT_DEFINE_IS_INTEGRAL(U16);
PJIT_DEFINE_IS_INTEGRAL(S16);
PJIT_DEFINE_IS_INTEGRAL(U32);
PJIT_DEFINE_IS_INTEGRAL(S32);
PJIT_DEFINE_IS_INTEGRAL(U64);
PJIT_DEFINE_IS_INTEGRAL(S64);
#undef PJIT_DEFINE_IS_INTEGRAL


template <typename T>
struct RemoveConst {
  typedef T Type;
};


template <typename T>
struct RemoveConst<const T> {
  typedef T Type;
};

}  // namespace pjit


#endif  // PJIT_BASE_TYPE_TRAITS_H_
