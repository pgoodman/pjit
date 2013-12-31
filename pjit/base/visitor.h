/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * visitor.h
 *
 *  Created on: 2013-12-30
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_VISITOR_H_
#define PJIT_BASE_VISITOR_H_

namespace pjit {

// Forward declaration for a visitor for a type. If this type is not
// specialized, then the default visitor type is treated as a function.
template <typename T>
struct VisitorFor {
 public:
  typedef void (Type)(T *);
};

}  // namespace pjit

#endif  // PJIT_BASE_VISITOR_H_
