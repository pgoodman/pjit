/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * control-flow-graph.cc
 *
 *  Created on: 2013-12-25
 *      Author: Peter Goodman
 */

#include "pjit/mir/cfg/basic-block.h"
#include "pjit/mir/cfg/control-flow-graph.h"
#include "pjit/mir/cfg/conditional.h"
#include "pjit/mir/cfg/finder.h"
#include "pjit/mir/cfg/multi-way-branch.h"
#include "pjit/mir/cfg/sequential.h"
#include "pjit/mir/cfg/loop.h"

namespace pjit {
namespace mir {

ControlFlowGraph::ControlFlowGraph(Context *context_, ControlFlowGraph *parent_)
    : context(context_),
      parent(parent_),
      last_visitor(nullptr) {}


ControlFlowGraphVisitor::ControlFlowGraphVisitor(void)
    : find_successors(nullptr),
      find_predecessors(nullptr) {}


#define PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(cls) \
  void ControlFlowGraphVisitor::VisitPreOrder(cls *cfg) { \
    if (cfg) { \
      cfg->VisitPreOrder(this); \
    } \
  } \
  void ControlFlowGraphVisitor::VisitPostOrder(cls *cfg) { \
    if (cfg) { \
      cfg->VisitPostOrder(this); \
    } \
  }


PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(SequentialControlFlowGraph)
PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(ConditionalControlFlowGraph)
PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(MultiWayBranchControlFlowGraph)
PJIT_DEFINE_CONTROL_FLOW_GRAPH_VISITORS(LoopControlFlowGraph)


void ControlFlowGraphVisitor::VisitSuccessors(BasicBlockVisitor *visitor) {
  if (find_successors) {
    find_successors->Visit(visitor);
  }
}


void ControlFlowGraphVisitor::VisitPredecessors(BasicBlockVisitor *visitor) {
  if (find_predecessors) {
    find_predecessors->Visit(visitor);
  }
}

}  // namespace mir
}  // namespace pjit
