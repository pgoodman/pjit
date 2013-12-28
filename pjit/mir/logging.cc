/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * log.cc
 *
 *  Created on: 2013-12-27
 *      Author: Peter Goodman
 */

#include "pjit/base/base.h"
#include "pjit/base/unsafe-cast.h"
#include "pjit/base/type-info.h"

#include "pjit/mir/logging.h"
#include "pjit/mir/context.h"
#include "pjit/mir/control-flow-graph.h"

namespace pjit {
namespace mir {


// Basic block visitor that prints the edges between two basic blocks.
class EdgeVisitor : public BasicBlockVisitor {
 public:
  EdgeVisitor(LogLevel level_, const BasicBlock *source_)
      : BasicBlockVisitor(),
        level(level_),
        source(source_),
        num_logged_bytes(0) {}

  virtual void Visit(BasicBlock *bb) {
    num_logged_bytes += Log(
        level,
        "b%p -> b%p;\n",
        reinterpret_cast<const void *>(source),
        reinterpret_cast<const void *>(bb));
  }

  int GetNumLoggedBytes(void) const {
    return num_logged_bytes;
  }

 private:
  const LogLevel level;
  const BasicBlock *source;
  int num_logged_bytes;
};


// Control-flow graph visitor that first prints out the instructions of a basic
// block (grouped into a DOT node), and then prints out edges between basic
// blocks.
class LoggerVisitor : public ControlFlowGraphVisitor {
 public:
  explicit LoggerVisitor(LogLevel level_)
      : ControlFlowGraphVisitor(),
        level(level_),
        num_logged_bytes(0) {};

  virtual ~LoggerVisitor(void) = default;

  virtual void VisitPreOrder(BasicBlock *bb) {
    // Print out instructions within a basic block.
    num_logged_bytes += Log(
        level, "b%p [label=\"", reinterpret_cast<const void *>(bb));

    for (Instruction *in(bb->first); nullptr != in; in = in->next) {
      num_logged_bytes += Log(level, in);
      if (in->next) {
        num_logged_bytes += Log(level, "\\l");
      }
    }
    num_logged_bytes += Log(level, "\"];\n");

    // Print out edges between basic blocks.
    EdgeVisitor edges(level, bb);
    VisitSuccessors(&edges);
    num_logged_bytes += edges.GetNumLoggedBytes();
  }

  int GetNumLoggedBytes(void) const {
    return num_logged_bytes;
  }

 private:
  const LogLevel level;
  int num_logged_bytes;

  LoggerVisitor(void) = delete;
  PJIT_DISALLOW_COPY_AND_ASSIGN(LoggerVisitor);
};

}  // namespace mir


// Logs out the control-flow graph represented by a MIR context as a DOT
// digraph.
int Log(LogLevel level, const mir::Context *context) {
  if (!context) {
    return 0;
  }

  int num_logged_bytes(0);
  num_logged_bytes += Log(level, "digraph {\nnode [shape=box, fontname=courier];\n");
  mir::LoggerVisitor logger(level);
  const_cast<mir::Context *>(context)->VisitPreOrder(&logger);
  num_logged_bytes += logger.GetNumLoggedBytes();
  num_logged_bytes += Log(level, "}\n");
  return num_logged_bytes;
}


// Log a pointer type.
static int Log(LogLevel level, const PointerTypeInfo *type) {
  int num_logged_bytes(0);
  num_logged_bytes += Log(level, type->pointed_to_type);
  num_logged_bytes += Log(level, "%s", type->info.name);
  return num_logged_bytes;
}


// Log a function type.
static int Log(LogLevel level, const FunctionTypeInfo *type) {
  int num_logged_bytes(0);
  num_logged_bytes += Log(level, type->return_type);
  num_logged_bytes += Log(level, "(*)(");
  for (unsigned i(0); i < type->num_arguments; ++i) {
    if (i) {
      num_logged_bytes += Log(level, ", ");
    }
    num_logged_bytes += Log(level, &(type->argument_types[i]));
  }
  num_logged_bytes += Log(level, ")");
  return num_logged_bytes;
}


// Log out a type.
int Log(LogLevel level, const TypeInfo *type) {
  switch (type->kind) {
    case TypeKind::TYPE_KIND_UNDEFINED:
    case TypeKind::TYPE_KIND_INTEGER:
    case TypeKind::TYPE_KIND_BOOLEAN:
    case TypeKind::TYPE_KIND_FLOATING_POINT:
      return Log(level, "%s", type->name);

    case TypeKind::TYPE_KIND_POINTER:
      return Log(level, UnsafeCast<const PointerTypeInfo *>(type));

    case TypeKind::TYPE_KIND_FUNCTION: {
      return Log(level, UnsafeCast<const FunctionTypeInfo *>(type));
      break;
    }

    case TypeKind::TYPE_KIND_STRUCTURE:
    case TypeKind::TYPE_KIND_UNION:
      break;
  }
  return 0;
}


// Log out a symbol, which might have a name/number, or might be an immediate.
// This also logs out the symbols type, as well as a suffix to log after the
// symbol.
static int Log(LogLevel level, const Symbol *sym, const char *suffix) {
  if (!sym) {
    return Log(level, "???");
  }

  int num_logged_bytes(0);
  num_logged_bytes += Log(level, sym->type);
  if (!sym->id) {  // Constant / immediate literal.
    return Log(level, ":C%s", suffix); // TODO(pag): Implement this.
  } else {
    num_logged_bytes += Log(
        level, ":%s$%u%s", sym->value.name, sym->id, suffix);
  }

  return num_logged_bytes;
}


// Logs out an individual MIR instruction.
int Log(LogLevel level, const mir::Instruction *in) {
  if (!in) {
    return 0;
  }

  int num_logged_bytes(0);

  switch (in->operation) {
#define PJIT_DECLARE_BINARY_OPERATOR(opcode, _) \
  case mir::Operation::PJIT_CAT(kOp, opcode): { \
    num_logged_bytes += Log(level, PJIT_TO_STRING(opcode) " "); \
    goto three_operands; \
  }
#define PJIT_DECLARE_UNARY_OPERATOR(opcode, _) \
  case mir::Operation::PJIT_CAT(kOp, opcode): { \
    num_logged_bytes += Log(level, PJIT_TO_STRING(opcode) " "); \
    num_logged_bytes += Log(level, in->operands[0], " <- "); \
    num_logged_bytes += Log(level, in->operands[1], ";"); \
    break; \
  }
#include "pjit/mir/operator.h"
#undef PJIT_DECLARE_BINARY_OPERATOR
#undef PJIT_DECLARE_UNARY_OPERATOR
    case mir::Operation::kOpLoadMemory: {
      num_logged_bytes += Log(level, "LoadMemory ");
      goto two_operands;
    }
    case mir::Operation::kOpStoreMemory: {
      num_logged_bytes += Log(level, "StoreMemory ");
      goto two_operands;
    }
    case mir::Operation::kOpConvertType: {
      num_logged_bytes += Log(level, "ConvertType ");
      goto two_operands;
    }
    case mir::Operation::kOpLoadImmediate: {
      num_logged_bytes += Log(level, "LoadImmediate ");
      goto two_operands;
    }
    case mir::Operation::kOpAssign: {
      num_logged_bytes += Log(level, "Assign ");
      goto two_operands;
    }
    default: {
      num_logged_bytes += Log(level, "UNKNOWN INSTRUCTION");
      break;
    }
  }

  two_operands:
  num_logged_bytes += Log(level, in->operands[0], " <- ");
  num_logged_bytes += Log(level, in->operands[1], ";");

  goto done;
  three_operands:
  num_logged_bytes += Log(level, in->operands[0], " <- ");
  num_logged_bytes += Log(level, in->operands[1], ", ");
  num_logged_bytes += Log(level, in->operands[2], ";");
  done:
  return num_logged_bytes;
}

}  // namespace pjit
