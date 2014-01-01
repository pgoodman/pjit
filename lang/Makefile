

include ../Makefile.inc


# Objects of the JIT to compile.
PJIT_LANG_OBJS = 
PJIT_LANG_OBJS += $(PJIT_BIN_DIR)/lang/basic-register-machine.o


# Compile C++ files.
$(PJIT_BIN_DIR)/lang/%.o : $(PJIT_SRC_DIR)/lang/%.cc
	@echo "Building CXX object $@"
	@mkdir -p $(@D)
	$(PJIT_CXX) $(PJIT_CXX_FLAGS) -c $< -o $@


all: $(PJIT_LANG_OBJS) $(PJIT_BIN_DIR)/pjit.o
	@$(PJIT_CXX) -static -o $(PJIT_BIN_DIR)/lang.out $(PJIT_LANG_OBJS) $(PJIT_BIN_DIR)/pjit.o
	@echo "Exiting $(PJIT_SRC_DIR)/lang."


clean:
	@echo "Cleaning $(PJIT_BIN_DIR)/lang"
	@rm -rf $(PJIT_LANG_OBJS) $(PJIT_BIN_DIR)/lang.out
	@echo "Exiting $(PJIT_SRC_DIR)/lang."
