/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * compiler.h
 *
 *  Created on: 2013-12-23
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_COMPILER_H_
#define PJIT_BASE_COMPILER_H_

namespace pjit {

#if PJIT_COMPILER_ICC
template <typename T>
struct AlignOf {
 public:
  struct AlignOfImpl {
    char _;
    T value;
  };

  enum {
    RESULT = __builtin_offsetof(AlignOfImpl, value)
  };
};


# define PJIT_ALIGNMENT_OF(type) \
  static_cast<unsigned>(pjit::AlignOf<type>::RESULT)
#else
# define PJIT_ALIGNMENT_OF(x) alignof(x)
#endif


#define PJIT_LIKELY(x) __builtin_expect((x),1)
#define PJIT_UNLIKELY(x) __builtin_expect((x),0)


}  // namespace pjit


#endif  // PJIT_BASE_COMPILER_H_
