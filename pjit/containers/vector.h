/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * vector.h
 *
 *  Created on: 2013-12-29
 *      Author: Peter Goodman
 */

#ifndef PJIT_CONTAINERS_VECTOR_H_
#define PJIT_CONTAINERS_VECTOR_H_

#include <new>

#include "pjit/base/base.h"
#include "pjit/base/compiler.h"
#include "pjit/base/unsafe-cast.h"

namespace pjit {

class GenericVectorPage;
template <typename T> class Vector;


// A generic vector implementation, based on page-granularity allocations. The
// purpose of the generic vector is to
class GenericVector {
 private:
  template <typename> friend class Vector;

  const unsigned object_size;
  const unsigned object_align;
  void (* const object_constructor)(void *);

  unsigned scale;

  GenericVectorPage *pages;

  GenericVector(void) = delete;
  GenericVector(unsigned object_size_, unsigned object_align_,
                void (*object_constructor_)(void *));
  ~GenericVector(void);

  GenericVectorPage *AllocateSlab(const unsigned scale,
                                  const unsigned start_index) const;

  void ConstructEntries(GenericVectorPage *page, unsigned from, unsigned to);
  void FreeSlab(GenericVectorPage *slab);

  void *Get(unsigned index);

  PJIT_DISALLOW_COPY_AND_ASSIGN(GenericVector);
};


template <typename T>
class Vector {
 public:
  Vector(void)
     : vector(sizeof(T), PJIT_ALIGNMENT_OF(T), &construct) {}

  T &Get(unsigned index) {
    return *UnsafeCast<T *>(vector.Get(index));
  }

  void Set(unsigned index, T &&value) {
    Get(index) = value;
  }

 private:
  GenericVector vector;

  static void construct(void *mem) {
    new (mem) T;
  }

  PJIT_DISALLOW_COPY_AND_ASSIGN_TEMPLATE(Vector, (T));
};

}  // namespace pjit

#endif  // PJIT_CONTAINERS_VECTOR_H_
