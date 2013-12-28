/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * finder.h
 *
 *  Created on: 2013-12-28
 *      Author: Peter Goodman
 */

#ifndef PJIT_MIR_CFG_FINDER_H_
#define PJIT_MIR_CFG_FINDER_H_

#include <initializer_list>

#include "pjit/base/base.h"

namespace pjit {
namespace mir {

class ControlFlowGraph;
class BasicBlockFinder;
class BasicBlockVisitor;
class MultiWayBranchArm;

// Defines a dynamically scoped chain of basic block finders. This class is
// responsible for saving a particular block finder, updating it, and then
// restoring the saved finder on destruction.
class BasicBlockFinderChain {
 public:
  BasicBlockFinderChain(BasicBlockFinder *&ptr_ref, BasicBlockFinder &curr);
  ~BasicBlockFinderChain(void);

 private:
  BasicBlockFinder **ptr;
  BasicBlockFinder *prev_value;

  BasicBlockFinderChain(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlockFinderChain);
};


// A generic basic block finder for finding some set of basic blocks.
class BasicBlockFinder {
 public:
  BasicBlockFinder(void) = default;
  virtual ~BasicBlockFinder(void) = default;
  virtual void Visit(BasicBlockVisitor *visitor) = 0;

 private:
  PJIT_DISALLOW_COPY_AND_ASSIGN(BasicBlockFinder);
};


// A generic basic block finder for finding some set of basic blocks.
class ArrayBasicBlockFinder : public BasicBlockFinder {
 public:
  explicit ArrayBasicBlockFinder(std::initializer_list<ControlFlowGraph *> args);
  virtual ~ArrayBasicBlockFinder(void) = default;
  virtual void Visit(BasicBlockVisitor *visitor) = 0;

 protected:
  enum {
    kMaxNumCfgs = 2
  };

  ControlFlowGraph *cfgs[kMaxNumCfgs];

 private:
  ArrayBasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(ArrayBasicBlockFinder);
};


// A basic block finder that, given a control flow graph, finds the entry basic
// blocks to the control flow graphs and invokes a visitor on those entry
// blocks.
class FirstBasicBlockFinder : public ArrayBasicBlockFinder {
 public:
  explicit FirstBasicBlockFinder(
      std::initializer_list<ControlFlowGraph *> args);

  virtual ~FirstBasicBlockFinder(void) = default;
  virtual void Visit(BasicBlockVisitor *visitor);

 private:
  FirstBasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(FirstBasicBlockFinder);
};


// A basic block finder that visits the predecessors of a specific control-flow
// graph.
class PredecessorBasicBlockFinder : public ArrayBasicBlockFinder {
 public:
  explicit PredecessorBasicBlockFinder(
      ControlFlowGraph *search_,
      std::initializer_list<ControlFlowGraph *> args);

  virtual ~PredecessorBasicBlockFinder(void) = default;
  virtual void Visit(BasicBlockVisitor *visitor);

 private:
  ControlFlowGraph *search;

  PredecessorBasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(PredecessorBasicBlockFinder);
};


// A basic block finder that visits the predecessors of a specific control-flow
// graph.
class MultiWayFirstBasicBlockFinder : public BasicBlockFinder {
 public:
  explicit MultiWayFirstBasicBlockFinder(MultiWayBranchArm *arms_);
  virtual ~MultiWayFirstBasicBlockFinder(void) = default;
  virtual void Visit(BasicBlockVisitor *visitor);

 private:
  MultiWayBranchArm *arms;

  MultiWayFirstBasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(MultiWayFirstBasicBlockFinder);
};


// A basic block finder that visits the predecessors of a specific control-flow
// graph.
class MultiWayPredecessorBasicBlockFinder : public BasicBlockFinder {
 public:
  MultiWayPredecessorBasicBlockFinder(
      ControlFlowGraph *search_, MultiWayBranchArm *arms_);

  virtual ~MultiWayPredecessorBasicBlockFinder(void) = default;
  virtual void Visit(BasicBlockVisitor *visitor);

 private:
  ControlFlowGraph *search;
  MultiWayBranchArm *arms;

  MultiWayPredecessorBasicBlockFinder(void) = delete;

  PJIT_DISALLOW_COPY_AND_ASSIGN(MultiWayPredecessorBasicBlockFinder);
};


}  // namespace mir
}  // namespace pjit


#endif  // PJIT_MIR_CFG_FINDER_H_
