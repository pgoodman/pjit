/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * vector.cc
 *
 *  Created on: 2013-12-29
 *      Author: Peter Goodman
 */

#include "pjit/base/compiler.h"
#include "pjit/base/memory.h"
#include "pjit/base/libc.h"
#include "pjit/base/numeric-types.h"
#include "pjit/containers/vector.h"

namespace pjit {


class GenericVectorPage {
 public:
  GenericVectorPage *next;
  unsigned min_index;  // Inclusive.
  unsigned max_index;  // Exclusive.
  unsigned max_assigned_index;
  unsigned order;
  UnsignedPointer object_pointer;
};


enum {
  // Do not cache generic vector slab allocations that have greater than or
  // equal to 2^kForceRetireMinScale pages.
  kForceRetireMinScale = 3,

  // Value with which to poison slab memory.
  kPoisonValue = 0xAB
};


// A cache of free pages available to any generic vector.
GenericVectorPage *PAGE_CACHE[] = {
  nullptr,  // 1 page.
  nullptr,  // 2 pages.
  nullptr  // 4 pages.
};


GenericVector::GenericVector(unsigned object_size_, unsigned object_align_,
                             void (*object_constructor_)(void *))
    : object_size(object_size_),
      object_align(object_align_),
      object_constructor(object_constructor_),
      scale(0),
      pages(nullptr) {}


GenericVector::~GenericVector(void) {
  for (GenericVectorPage *next(nullptr); nullptr != pages; pages = next) {
    next = pages->next;
    FreeSlab(pages);
  }
}


// Allocate a new memory slab for a generic allocator.
GenericVectorPage *GenericVector::AllocateSlab(
    const unsigned alloc_scale, const unsigned start_index) const {

  GenericVectorPage *page(nullptr);
  if (scale < kForceRetireMinScale && nullptr != PAGE_CACHE[scale]) {
    page = PAGE_CACHE[alloc_scale];
    PAGE_CACHE[alloc_scale] = page->next;
  }

  const unsigned num_page_frames(1 << alloc_scale);
  if (!page) {
    page = UnsafeCast<GenericVectorPage *>(AllocatePages(num_page_frames));
    ProtectPages(
        page, num_page_frames, MemoryProtection::MEMORY_READ_WRITE);
  }

  const unsigned num_bytes(num_page_frames * PAGE_FRAME_SIZE);
  memset(page, kPoisonValue, num_bytes);

  const UnsignedSize aligned_page_size(
      PJIT_ALIGN_TO(sizeof(GenericVectorPage),
                    static_cast<UnsignedSize>(object_align)));
  const UnsignedSize max_available_bytes(num_bytes - aligned_page_size);
  const UnsignedSize max_available_objects(max_available_bytes / object_size);

  page->object_pointer = UnsafeCast<UnsignedPointer>(page) + aligned_page_size;
  page->min_index = start_index;
  page->max_index = static_cast<unsigned>(start_index + max_available_objects);
  page->max_assigned_index = start_index;
  page->order = scale;
  page->next = nullptr;

  return page;
}


// Free a slab. This will either place the slab back on a free list and protect
// the slab from reads/writes, or it will free the slab back to the OS.
void GenericVector::FreeSlab(GenericVectorPage *slab) {
  const unsigned num_page_frames(1 << slab->order);
  if (slab->order < kForceRetireMinScale) {
    slab->next = PAGE_CACHE[slab->order];
    PAGE_CACHE[slab->order] = slab;
    ProtectPages(
        slab, num_page_frames, MemoryProtection::MEMORY_INACCESSIBLE);
  } else {
    FreePages(slab, num_page_frames);
  }
}


void GenericVector::ConstructEntries(GenericVectorPage *page, unsigned from,
                                     unsigned to) {
  for (UnsignedPointer i(page->object_pointer + from * object_size),
                       max_i(page->object_pointer + to * object_size);
      i < max_i; i += object_size) {
    object_constructor(UnsafeCast<void *>(i));
  }
}


void *GenericVector::Get(unsigned index) {
  if (PJIT_UNLIKELY(!pages)) {
    pages = AllocateSlab(scale++, 0);
  }

  for (;;) {
    for (GenericVectorPage *page(pages);
         PJIT_LIKELY(nullptr != page);
         page = page->next) {

     if (PJIT_LIKELY(index < page->max_index)) {  // In this page.
        if (PJIT_UNLIKELY(index < page->min_index)) {  // In another page.
          continue;
        }

        const unsigned relative_index(index - page->min_index);
        const unsigned relative_offset(relative_index * object_size);

        if (PJIT_UNLIKELY(index > page->max_assigned_index)) {
          ConstructEntries(
              page,
              page->max_assigned_index - page->min_index,
              relative_index + 1);

          page->max_assigned_index = index;
        }

        return UnsafeCast<void *>(page->object_pointer + relative_offset);

      } else {  // Need to allocate a new page.
        page = AllocateSlab(scale++, page->max_index);
        page->next = pages;
        pages = page;
        break; // go around the outer loop again.
      }
    }
  }

  return nullptr;
}

}  // namespace pjit
