

include ../Makefile.inc

PJIT_CXX_FLAGS += -fno-asynchronous-unwind-tables -fno-rtti -nostdlib 
PJIT_CXX_FLAGS += -mno-red-zone -fno-stack-protector -fno-common -fno-builtin
PJIT_CXX_FLAGS += -ffreestanding -fno-exceptions -fno-math-errno

# Disable a number of features for the JIT components. We care about disabling
# a number of optimizations and language features that might affect certain
# register usage. We also care about having as little dependency on libc and
# libc++ as possible.
ifeq ($(PJIT_GCC),1)
    PJIT_CXX_FLAGS += -nostartfiles -nodefaultlibs
endif

# Objects of the JIT to compile.
PJIT_OBJS = 
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/base/type-info.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/base/allocate.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/base/logging.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/context.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/cfg/control-flow-graph.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/cfg/finder.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/cfg/basic-block.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/cfg/sequential.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/cfg/conditional.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/cfg/multi-way-branch.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/mir/logging.o
PJIT_OBJS += $(PJIT_BIN_DIR)/pjit/hir/hir-to-mir.o


# Mask some of the PJIT symbols.
PJIT_LD_FLAGS =
PJIT_LD_FLAGS += --defsym=memset=pjit_memset
PJIT_LD_FLAGS += --defsym=memcpy=pjit_memcpy
PJIT_LD_FLAGS += --defsym=memcmp=pjit_memcmp


PJIT_LINT_DIR = $(PJIT_SRC_DIR)/dependencies/cpplint
PJIT_LINT_FLAGS = --root=$(PJIT_SRC_DIR) --verbose=5
PJIT_LINT = $(PJIT_PYTHON) $(PJIT_LINT_DIR)/cpplint.py $(PJIT_LINT_FLAGS)

# Dependencies.
#PJIT_DEPS := $(PJIT_OBJS:.o=.d)
#-include $(PJIT_DEPS)

# Compile C++ file dependencies.
#$(PJIT_BIN_DIR)/pjit/%.d : $(PJIT_SRC_DIR)/pjit/%.cc
#	@echo "Computing dependencies for $<"
#	@mkdir -p $(@D)
#	@$(PJIT_CXX) $(PJIT_CXX_FLAGS) -MP -MT $(patsubst %.d,%.o,$@) -MM $^ -MF $@

.PHONY: all lint clean

# Compile C++ files.
$(PJIT_BIN_DIR)/pjit/%.o : $(PJIT_SRC_DIR)/pjit/%.cc
	@echo "Building CXX object $@"
	@mkdir -p $(@D)
	@$(PJIT_CXX) $(PJIT_CXX_FLAGS) -c $< -o $@


lint:
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/base/*.h)
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/base/*.cc)
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/hir/*.h)
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/hir/*.cc)
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/mir/*.h)
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/mir/*.cc)
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/mir/cfg/*.h)
	$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/mir/cfg/*.cc)
	#$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/*.h)
	#$(PJIT_LINT) $(wildcard $(PJIT_SRC_DIR)/pjit/*.cc)

all: lint $(PJIT_OBJS)
	@echo "Entering $(PJIT_SRC_DIR)/pjit/arch"
	$(MAKE) -C $(PJIT_SRC_DIR)/pjit/arch $(MFLAGS) all
	@$(PJIT_LD) $(PJIT_LD_FLAGS) -r -o $(PJIT_BIN_DIR)/pjit.o $(PJIT_OBJS) $(PJIT_BIN_DIR)/arch.o
	@echo "Exiting $(PJIT_SRC_DIR)/pjit."


clean:
	@echo "Entering $(PJIT_SRC_DIR)/pjit/arch"
	$(MAKE) -C $(PJIT_SRC_DIR)/pjit/arch $(MFLAGS) clean
	@echo "Cleaning $(PJIT_BIN_DIR)/pjit"
	@rm -rf $(PJIT_OBJS)
	@rm -rf $(PJIT_DEPS)
	@rm -rf $(PJIT_BIN_DIR)/pjit.o
	@echo "Exiting $(PJIT_SRC_DIR)/pjit."
