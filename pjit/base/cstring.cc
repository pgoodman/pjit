/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * cstring.cc
 *
 *  Created on: 2013-12-31
 *      Author: Peter Goodman
 */

#include "pjit/base/compiler.h"

namespace pjit {

bool CStringsAreEqual(const char *safe_string, const char *test_string) {
  if (safe_string == test_string) {
    return true;
  }

  if (PJIT_LIKELY(nullptr != safe_string)) {
    if (PJIT_UNLIKELY(!test_string)) {
      return false;
    }
  } else if (PJIT_LIKELY(nullptr != test_string)) {
    return false;
  }

  // Both are non-nullptr.
  for (; *safe_string == *test_string; safe_string++, test_string++) {
    if (!*safe_string) {
      return true;
    }
  }

  return false;
}

}  // namespace pjit


