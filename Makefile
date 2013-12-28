
include Makefile.inc

.PHONY: all clean


all:
	@echo "Entering $(PJIT_SRC_DIR)/pjit"
	$(MAKE) -C $(PJIT_SRC_DIR)/pjit $(MFLAGS) all
	@echo "Entering $(PJIT_SRC_DIR)/lang"
	$(MAKE) -C $(PJIT_SRC_DIR)/lang $(MFLAGS) all

clean:
	@echo "Entering $(PJIT_SRC_DIR)/pjit"
	$(MAKE) -C $(PJIT_SRC_DIR)/pjit $(MFLAGS) clean
	@echo "Entering $(PJIT_SRC_DIR)/lang"
	$(MAKE) -C $(PJIT_SRC_DIR)/lang $(MFLAGS) clean
