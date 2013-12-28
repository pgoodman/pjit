/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * log.h
 *
 *  Created on: 2013-12-27
 *      Author: Peter Goodman
 */

#ifndef PJIT_BASE_LOGGING_H_
#define PJIT_BASE_LOGGING_H_

namespace pjit {

enum class LogLevel : unsigned {
  LogOutput = 0,
  LogWarning = 1,
  LogError = 2,
  LogFatalError = 3
};

int Log(LogLevel, const char *, ...) __attribute__ ((format (printf, 2, 3)));

}  // namespace pjit

#endif  // PJIT_BASE_LOGGING_H_
