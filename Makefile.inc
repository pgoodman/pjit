
PJIT_TARGET ?= debug

PJIT_LD ?= ld
PJIT_CC ?= clang
PJIT_CXX ?= clang++
PJIT_PYTHON ?= python2.7

PJIT_EXTRA_CXX_FLAGS ?=

PJIT_SRC_DIR ?= /home/pag/Code/PJIT
PJIT_BIN_DIR ?= /home/pag/Code/PJIT/bin

# Make the binary directory target-specific.
PJIT_BIN_DIR := $(PJIT_BIN_DIR)/$(PJIT_TARGET)

PJIT_LD_FLAGS_EARLY =
PJIT_LD_FLAGS_LATE =

PJIT_CXX_FLAGS = $(PJIT_EXTRA_CXX_FLAGS)

# Include path. All source files should use relative imports, with respect to
# `PJIT_SRC_DIR`.
PJIT_CXX_FLAGS += -I$(PJIT_SRC_DIR)

# Use C++11.
PJIT_CXX_FLAGS += -std=c++11

# Detect the compiler being used.
PJIT_GCC ?= 0
PJIT_CLANG ?= 1
PJIT_ICC ?= 0
PJIT_GCC_COMPAT ?=0

# Enable/disable features based on whether or not this is a debug or release
# build.
ifeq ($(PJIT_TARGET),debug)
    PJIT_CXX_FLAGS += -O0 -g3
    #PJIT_CXX_FLAGS += -fsanitize=undefined
    #PJIT_CXX_FLAGS += -fno-sanitize=vptr
else
    PJIT_CXX_FLAGS += -O3 -g0
endif

# Enable various warnings and errors.
PJIT_CXX_FLAGS += -Wall -Wextra -Werror -pedantic -pedantic-errors
PJIT_CXX_FLAGS += -Wno-variadic-macros -Wcast-qual -Wshadow -Wpointer-arith
PJIT_CXX_FLAGS += -Wconversion -Wuninitialized -Winit-self 
PJIT_CXX_FLAGS += -Wmissing-declarations -Wwrite-strings -Wfloat-equal

PJIT_CXX_FLAGS += -fstrict-aliasing -Wstrict-aliasing=2

ifeq (0,$(PJIT_ICC))
    PJIT_CXX_FLAGS += -Wstrict-overflow=2
    PJIT_CXX_FLAGS += -I/usr/include/x86_64-linux-gnu/c++/4.8/ # TODO
else
    # offsetof applied to non-POD (Plain Old Data) types is nonstandard
    PJIT_CXX_FLAGS += -diag-disable 1875
    
    # defining no constructors to initialize fields in a C-style struct.
    PJIT_CXX_FLAGS += -diag-disable 411
    
    # 'declaration hides parameter' issue with variadic templates.
    PJIT_CXX_FLAGS += -diag-disable 1599
    
    # 'implicit conversion of a 64-bit integral type to a smaller integral type'
    PJIT_CXX_FLAGS += -diag-disable 1682
    
    #PJIT_CXX_FLAGS += -Kc++
    
    # Find the standard library.
    PJIT_LIBCXX_DIR = $(shell locate c++config.h | grep /usr | tail -n 1)
    ifneq (,$(PJIT_LIBCXX_DIR))
        PJIT_CXX_FLAGS += -I$(dir $(realpath $(dir $(PJIT_LIBCXX_DIR))))
    endif
endif
ifeq (1,$(PJIT_CLANG))
    PJIT_GCC_COMPAT = 1
    PJIT_CXX_FLAGS += -ferror-limit=10 -Wno-gnu 
endif
ifeq (1,$(PJIT_GCC))
    PJIT_GCC_COMPAT = 1
endif

# Enable warnings that are targeted at very GNU-like compilers (GCC, Clang)
ifeq (1,$(PJIT_GCC_COMPAT))
    PJIT_CXX_FLAGS += -Wcast-align -Wredundant-decls -Wvolatile-register-var
    PJIT_CXX_FLAGS += -Wstack-protector -Wold-style-cast
endif


# Tell PJIT which compiler is being used.
PJIT_CXX_FLAGS += -DPJIT_COMPILER_ICC=$(PJIT_ICC)
PJIT_CXX_FLAGS += -DPJIT_COMPILER_GCC=$(PJIT_GCC)
PJIT_CXX_FLAGS += -DPJIT_COMPILER_CLANG=$(PJIT_CLANG)

.SILENT :
