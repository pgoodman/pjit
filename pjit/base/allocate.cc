/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * allocate.cc
 *
 *  Created on: 2013-12-22
 *      Author: Peter Goodman
 */


#include "pjit/base/allocate.h"
#include "pjit/base/libc.h"

#include <sys/mman.h>

#define PROT_ALL (~0)
#ifndef MAP_ANONYMOUS
# ifdef MAP_ANON
#   define MAP_ANONYMOUS MAP_ANON
# else
#   define MAP_ANONYMOUS 0
# endif
#endif
#ifndef MAP_SHARED
# define MAP_SHARED 0
#endif


namespace pjit {


// Allocates `num` number of pages from the OS with `MEMORY_READ_WRITE`
// protection.
void *AllocatePages(unsigned num) {
  void *ret(mmap(
      nullptr,
      PJIT_PAGE_FRAME_SIZE * num,
      PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANONYMOUS,
      -1,
      0));

  memset(ret, 0, PJIT_PAGE_FRAME_SIZE);

  return ret;
}


// Frees `num` pages back to the OS.
void FreePages(void *addr, unsigned num) {
  munmap(addr, num * PJIT_PAGE_FRAME_SIZE);
}


// Changes the memory protection of some pages.
void ProtectPages(void *addr, unsigned num, MemoryProtection prot) {
  int prot_bits(0);
  if (MemoryProtection::MEMORY_EXECUTABLE == prot) {
    prot_bits = PROT_EXEC;
  } else if (MemoryProtection::MEMORY_READ_ONLY == prot) {
    prot_bits = PROT_READ;
  } else {
    prot_bits = PROT_READ | PROT_WRITE;
  }
  mprotect(addr, num * PJIT_PAGE_FRAME_SIZE, prot_bits);
}


}  // namespace pjit
