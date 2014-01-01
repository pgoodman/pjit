/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * allocate.h
 *
 *  Created on: 2013-12-22
 *      Author: Peter Goodman
 */

#ifndef PJIT_CONTAINERS_ALLOCATOR_H_
#define PJIT_CONTAINERS_ALLOCATOR_H_

#include <new>  // NOLINT

#include "pjit/base/base.h"
#include "pjit/base/memory.h"
#include "pjit/base/compiler.h"
#include "pjit/base/numeric-types.h"
#include "pjit/base/libc.h"
#include "pjit/base/visitor.h"

namespace pjit {


// Defines a simple, single-threaded allocator for a single kind of object. The
// size of the allocated object must be strictly less than a page.
//
// TODO(pag): Eventually this should be improved. Some improvements that could
//            be made without changing too much:
//              1) Use a bitset instead of an array of `ObjectMetaData`.
//              2) Merge the `full_pages` and `partial_pages`, so that full
//                 pages are at the end of the list and partial pages are at
//                 the beginning.
template <typename T, unsigned kNumPages=1>
class Allocator {
 public:

  Allocator(void)
      : full_pages(nullptr),
        partial_pages(nullptr) {}

  ~Allocator(void) {
    FreeAll();
  }

  template <typename... Args>
  T *Allocate(Args... args) {
    T *obj(GetFreeObject());
    new (obj) T(args...);
    return obj;
  }

  void Free(T *obj) {
    if (!obj) {
      return;
    }

    obj->~T();

    memset(obj, POISON, sizeof *obj);  // Poison the memory;

    PageMetaData *page(ObjectToPage(obj));
    FreeFromPage(page, obj - page->objects);
  }

  void FreeAll(void) {
    FreePageList(full_pages);
    FreePageList(partial_pages);

    full_pages = nullptr;
    partial_pages = nullptr;
  }

  void MarkAllUnreachable(void) {
    MarkPagesUnreachable(full_pages);
    MarkPagesUnreachable(partial_pages);
  }

  void MarkReachable(const T *obj) {
    PageMetaData *page(ObjectToPage(obj));
    page->status[obj - page->objects].is_reachable = true;
  }

  void FreeUnreachable(void) {
    FreeUnreachableObjectsOnPages(full_pages);
    FreeUnreachableObjectsOnPages(partial_pages);

    for (PageMetaData *page(full_pages), *next(nullptr);
         nullptr != page; page = next) {
      next = page->next;

      if (page->num_free) {
        UnchainPage(full_pages, page);
        ChainPage(partial_pages, page);
      }
    }
  }

  // Determine whether some memory is owned by this allocator. This is somewhat
  // unsafe, as it assumes that the `allocator` pointer for the page meta-data
  // is consistently placed across different instantiations of the `Allocator`
  // class template.
  bool OwnsObject(const T *obj) {
    if (!obj) {
      return false;
    }
    return this == ObjectToPage(obj)->allocator;
  }


  // Apply a visitor to every allocated object owned by this allocator.
  void Visit(typename VisitorFor<T>::Type *visitor) {
    VisitPageList(full_pages, visitor);
    VisitPageList(partial_pages, visitor);
  }

 private:
  struct PageMetaData;

  // The basic implementation of the page meta-data. This excludes the actual
  // meta-data about individual objects.
  struct PageMetaDataImpl {

    // Used to detect if this allocator or another allocator owns this object.
    const void * const allocator;

    PageMetaData *prev;
    PageMetaData *next;
    unsigned num_allocated;
    unsigned num_free;
    T *objects;

    explicit PageMetaDataImpl(void *allocator_)
        : allocator(allocator_),
          prev(nullptr),
          next(nullptr),
          num_allocated(0),
          num_free(0),
          objects(nullptr) {}
  };

  enum : unsigned {
    SLAB_SIZE = pjit::PAGE_FRAME_SIZE * kNumPages,
    OBJECT_SIZE = sizeof(T),
    OBJECT_ALIGN = PJIT_ALIGNMENT_OF(T),
    IMPL_SIZE = sizeof(PageMetaDataImpl),
    ESTIMATED_NUM_OBJECTS = (SLAB_SIZE - IMPL_SIZE) / OBJECT_SIZE,
    POISON = 0xAB
  };

  // Meta-data about allocated objects. These bits determine whether or not
  // an object is allocated, as well as whether it has been marked as reachable
  // or not within the current garbage collection epoch.
  //
  // Note: Garbage collection epochs are externally defined, and interface with
  //       the allocator by means of the `MarkAllUnreachable`, `MarkReachable`,
  //       and `FreeUnreachable` methods.
  struct ObjectMetaData {
    bool is_allocated:1;
    bool is_reachable:1;
  };

  // Full page meta-data, including per-object meta-data.
  struct PageMetaData : public PageMetaDataImpl {
    ObjectMetaData status[ESTIMATED_NUM_OBJECTS];

    explicit PageMetaData(void *allocator_)
        : PageMetaDataImpl(allocator_) {
      memset(&(status[0]), 0, sizeof(ObjectMetaData) * ESTIMATED_NUM_OBJECTS);
    }
  };

  enum : unsigned {
    NEEDED_ALIGNMENT = (sizeof(PageMetaData) % OBJECT_ALIGN)
        ? OBJECT_ALIGN - (sizeof(PageMetaData) % OBJECT_ALIGN) : 0,

    BEGIN_OFFSET = sizeof(PageMetaData) + NEEDED_ALIGNMENT,

    // Maximum number of objects that can be allocated from a single Page
    // struct.
    NUM_OBJECTS = (SLAB_SIZE - BEGIN_OFFSET) / sizeof(T)
  };

  static_assert(OBJECT_SIZE < SLAB_SIZE, "Object to allocate is too big.");
  static_assert(0 < NUM_OBJECTS, "Object size is too big for the allocator.");
  static_assert(
      static_cast<unsigned>(NUM_OBJECTS) <=
      static_cast<unsigned>(ESTIMATED_NUM_OBJECTS),
      "The estimated maximum number of objects that can fit into a page of "
      "memory does not overestimate the actual maximum number of objects.");

  // Linked list of pages that are completely used.
  PageMetaData *full_pages;

  // Linked list of pages that are partiall used.
  PageMetaData *partial_pages;

  Allocator(const Allocator<T> &) = delete;
  Allocator(const Allocator<T> &&) = delete;

  // Returns the Page containing this object.
  PageMetaData *ObjectToPage(const T *object) {
    const UnsignedPointer addr(reinterpret_cast<UnsignedPointer>(object));
    return reinterpret_cast<PageMetaData *>(addr - (addr % SLAB_SIZE));
  }

  // Returns a new Page for use.
  PageMetaData *AllocatePage(void) {
    void *addr(AllocatePages(kNumPages));
    memset(addr, POISON, SLAB_SIZE);
    PageMetaData *page(new (addr) PageMetaData(this));
    page->num_free = NUM_OBJECTS;
    page->objects = reinterpret_cast<T *>(
        reinterpret_cast<UnsignedPointer>(addr) + BEGIN_OFFSET);
    return page;
  }

  // Unchain a page in a page list.
  void UnchainPage(PageMetaData *&list, PageMetaData *page) {
    PageMetaData *prev(page->prev);
    PageMetaData *next(prev->next);
    if (prev) {
      prev->next = next;
    } else {
      list = next;
    }

    if (next) {
      next->prev = prev;
    }

    page->next = nullptr;
    page->prev = nullptr;
  }

  // Chain a page into a page list.
  void ChainPage(PageMetaData *&list, PageMetaData *page) {
    if (list) {
      list->prev = page;
    }
    page->prev = nullptr;
    page->next = list;
    list = page;
  }

  // Returns a zero-initialized object from slot `i` of Page `page`. This will
  // return an otherwise uniniatlized object (i.e. the constructor is not
  // invoked here).
  T *AllocateFromPage(PageMetaData *page, unsigned i) {
    page->status[i].is_allocated = true;
    page->num_free -= 1;
    page->num_allocated += 1;

    if (!page->num_free) {
      UnchainPage(partial_pages, page);
      ChainPage(full_pages, page);
    }

    T *ret(&(page->objects[i]));
    memset(ret, 0, sizeof *ret);
    return ret;
  }

  // Free the object stored in slot `i` of the Page `page`.
  void FreeFromPage(PageMetaData *page, unsigned i) {

    page->status[i].is_allocated = false;
    page->num_allocated -= 1;

    if (!page->num_allocated) {
      UnchainPage(partial_pages, page);
      FreePages(page, kNumPages);

    } else if (!page->num_free) {
      UnchainPage(full_pages, page);
      ChainPage(partial_pages, page);
    }

    page->num_free += 1;
  }

  // Get an uninitialized (zero-initialized)
  T *GetFreeObject(void) {
    if (!partial_pages) {
      ChainPage(partial_pages, AllocatePage());
    }

    for (unsigned i(0); i < NUM_OBJECTS; ++i) {
      if (!partial_pages->status[i].is_allocated) {
        return AllocateFromPage(partial_pages, i);
      }
    }

    return nullptr;
  }

  void FreeObjectsOnPage(PageMetaData *page) {
    for (unsigned i(0); i < NUM_OBJECTS; ++i) {
      if (page->status[i].is_allocated) {
        page->status[i].is_allocated = false;
        page->objects[i].~T();
      }
    }
    page->num_allocated = 0;
    page->num_free = NUM_OBJECTS;
  }

  void FreeUnreachableObjectsOnPages(PageMetaData *page) {
    for (; nullptr != page; page = page->next) {
      for (unsigned i(0); i < NUM_OBJECTS; ++i) {
        if (page->status[i].is_allocated && !page->status[i].is_reachable) {
          T *object(&(page->objects[i]));
          page->status[i].is_allocated = false;
          object->~T();
          memset(object, POISON, sizeof(T));

          --(page->num_allocated);
          ++(page->num_free);
        }
      }
    }
  }

  void FreePageList(PageMetaData *list) {
    for (PageMetaData *next(nullptr); nullptr != list; list = next) {
      next = list->next;
      FreeObjectsOnPage(list);
      FreePages(list, kNumPages);
    }
  }

  void MarkPagesUnreachable(PageMetaData *page) {
    for (; nullptr != page; page = page->next) {
      for (unsigned i(0); i < NUM_OBJECTS; ++i) {
        if (page->status[i].is_allocated) {
          page->status[i].is_reachable = false;
        }
      }
    }
  }

  // Apply a visitor to every allocated object owned by this allocator.
  void VisitPageList(PageMetaData *page,
                     typename VisitorFor<T>::Type *visitor) {
    for (; nullptr != page; page = page->next) {
      for (unsigned i(0); i < NUM_OBJECTS; ++i) {
        if (page->status[i].is_allocated) {
          visitor->Visit(&(page->objects[i]));
        }
      }
    }
  }

  PJIT_DISALLOW_ASSIGN(Allocator<T>);
};

}  // namespace pjit


#endif  // PJIT_CONTAINERS_ALLOCATOR_H_
