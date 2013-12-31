/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * base.h
 *
 *  Created on: 2013-12-26
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_BASE_H_
#define PJIT_BASE_BASE_H_


#define PJIT_UNPACK(...) __VA_ARGS__


#define PJIT_ALIGN_FACTOR(x, align) \
  (((x) % (align)) ? ((x) - ((x) % (align))) : 0)


#define PJIT_ALIGN_TO(x, align) \
  ((x) + PJIT_ALIGN_FACTOR(x, align))


#define PJIT_DISALLOW_COPY(cls) \
  cls(const cls &) = delete; \
  cls(const cls &&) = delete


#define PJIT_DISALLOW_ASSIGN(cls) \
  cls &operator=(const cls &) = delete; \
  cls &operator=(const cls &&) = delete


#define PJIT_DISALLOW_COPY_AND_ASSIGN(cls) \
  PJIT_DISALLOW_COPY(cls); \
  PJIT_DISALLOW_ASSIGN(cls)


#define PJIT_DISALLOW_COPY_TEMPLATE(cls, params) \
  cls(const cls<PJIT_UNPACK params> &) = delete; \
  cls(const cls<PJIT_UNPACK params> &&) = delete


#define PJIT_DISALLOW_COPY_AND_ASSIGN_TEMPLATE(cls, params) \
  PJIT_DISALLOW_COPY_TEMPLATE(cls, params); \
  cls<PJIT_UNPACK params> &operator=(const cls<PJIT_UNPACK params> &) = delete; \
  cls<PJIT_UNPACK params> &operator=(const cls<PJIT_UNPACK params> &&) = delete


// Concatenate two pre-processor symbols into a single symbol.
#define PJIT_CAT3(a, b) a ## b
#define PJIT_CAT2(a, b) PJIT_CAT3(a, b)
#define PJIT_CAT(a, b) PJIT_CAT2(a, b)


#define PJIT_TO_STRING3(a) #a
#define PJIT_TO_STRING2(a) PJIT_TO_STRING3(a)
#define PJIT_TO_STRING(a) PJIT_TO_STRING2(a)

#define PJIT_UNUSED(a) (void) (a)

#endif  // PJIT_BASE_BASE_H_
