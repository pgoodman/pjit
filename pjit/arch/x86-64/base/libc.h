/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * libc.h
 *
 *  Created on: 2013-12-22
 *      Author: Peter Goodman
 */

#ifndef PJIT_ARCH_X86_64_BASE_LIBC_H_
#define PJIT_ARCH_X86_64_BASE_LIBC_H_


extern "C" {
  extern void *pjit_memcpy(void *, const void *, unsigned long);
  extern void *pjit_memset(void *, int, unsigned long);
  extern int pjit_memcmp(const void *, const void *, unsigned long);
}

#define memcpy pjit_memcpy
#define memset pjit_memset
#define memcmp pjit_memcmp

#endif  // PJIT_ARCH_X86_64_BASE_LIBC_H_
