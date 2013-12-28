/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * log.h
 *
 *  Created on: 2013-12-27
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_LOGGING_H_
#define PJIT_MIR_LOGGING_H_

#include "pjit/base/logging.h"

namespace pjit {

struct TypeInfo;

namespace mir {
class Context;
class Instruction;
}  // namespace mir


// Logs out the name of a type.
int Log(LogLevel level, const TypeInfo *type);


// Logs out the control-flow graph represented by a MIR context as a DOT
// digraph.
int Log(LogLevel, const mir::Context *);


// Logs out an individual MIR instruction.
int Log(LogLevel, const mir::Instruction *);

}  // namespace pjit

#endif  // PJIT_MIR_LOGGING_H_
