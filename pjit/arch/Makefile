

include ../../Makefile.inc

PJIT_ARCH_OBJS =
PJIT_ARCH_OBJS += $(PJIT_BIN_DIR)/pjit/arch/x86-64/base/libc.o


$(PJIT_BIN_DIR)/pjit/arch/%.o : $(PJIT_SRC_DIR)/pjit/arch/%.S
	@echo "Building AS object $<"
	@mkdir -p $(@D)
	@$(PJIT_CC) -c $< -o $@


all: $(PJIT_ARCH_OBJS)
	@$(PJIT_LD) $(PJIT_LD_FLAGS) -r -o $(PJIT_BIN_DIR)/arch.o $(PJIT_ARCH_OBJS)
	@echo "Exiting $(PJIT_SRC_DIR)/pjit/arch."


clean:
	@echo "Cleaning $(PJIT_BIN_DIR)/pjit/arch"
	@rm -rf $(PJIT_ARCH_OBJS)
	@echo "Exiting $(PJIT_SRC_DIR)/pjit/arch."