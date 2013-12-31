/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * gc.h
 *
 *  Created on: 2013-12-29
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_VISITORS_GARBAGE_COLLECT_VISIT_H_
#define PJIT_MIR_VISITORS_GARBAGE_COLLECT_VISIT_H_

#include "pjit/base/base.h"
#include "pjit/mir/cfg/control-flow-graph.h"

namespace pjit {
namespace mir {

class SequentialControlFlowGraph;
class ConditionalControlFlowGraph;
class MultiWayBranchControlFlowGraph;
class LoopControlFlowGraph;
class BasicBlock;


// Control-flow graph visitor that first prints out the instructions of a basic
// block (grouped into a DOT node), and then prints out edges between basic
// blocks.
class GarbageCollectionVisitor : public ControlFlowGraphVisitor {
 public:
  explicit GarbageCollectionVisitor(Context *context_);
  virtual ~GarbageCollectionVisitor(void) = default;
  virtual void VisitPreOrder(SequentialControlFlowGraph *cfg);
  virtual void VisitPreOrder(ConditionalControlFlowGraph *cfg);
  virtual void VisitPreOrder(MultiWayBranchControlFlowGraph *cfg);
  virtual void VisitPreOrder(LoopControlFlowGraph *cfg);
  virtual void VisitPreOrder(BasicBlock *bb);

 private:
  Context *context;

  GarbageCollectionVisitor(void) = delete;
  PJIT_DISALLOW_COPY_AND_ASSIGN(GarbageCollectionVisitor);
};

}  // namespace mir
}  // namespace pjit

#endif  // PJIT_MIR_VISITORS_GARBAGE_COLLECT_VISIT_H_
