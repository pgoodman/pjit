/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * memory.h
 *
 *  Created on: 2013-12-29
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_MEMORY_H_
#define PJIT_BASE_MEMORY_H_

namespace pjit {


enum {
  PAGE_FRAME_SIZE = 4096
};


// Defines the various kinds of available memory protection. This is not an
// exhaustive list, e.g. in practice, one could have all of read, write, and
// execute permissions; however, limiting to these three kinds of protections
// serves as a good discipline.
enum class MemoryProtection {
  MEMORY_EXECUTABLE,  // Implies read-only status.
  MEMORY_READ_ONLY,
  MEMORY_READ_WRITE,
  MEMORY_INACCESSIBLE
};


// Allocates `num` number of pages from the OS with `MEMORY_READ_WRITE`
// protection.
void *AllocatePages(unsigned num);


// Frees `num` pages back to the OS.
void FreePages(void *, unsigned num);


// Changes the memory protection of some pages.
void ProtectPages(void *addr, unsigned num, MemoryProtection prot);


}  // namespace pjit

#endif  // PJIT_BASE_MEMORY_H_
