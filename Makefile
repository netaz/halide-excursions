# Before running make you must set HALIDE_HOME to the directory where the Halide source code resides
HALIDE_HOME ?= HALIDE_HOME_MUST_BE_SET

CXX ?= g++
OPTIMIZE ?= -O3
CXX11 ?=

CXX_WARNING_FLAGS = -Wall -Werror -Wno-unused-function -Wcast-qual
CXX_FLAGS = $(CXX_WARNING_FLAGS) -fno-rtti -Woverloaded-virtual -fPIC $(OPTIMIZE) -fno-omit-frame-pointer
CXX_FLAGS += -std=c++0x

BUILD_DIR = build
BIN_DIR = bin
EXAMPLES_DIR = examples
GEN_DIR = generated

FUNCS_SRC = kernels/cv.cpp kernels/color_convert.cpp kernels/openvx.cpp
HEADER_FILES = ./utils/clock.h 

FUNCS_OBJ = $(FUNCS_SRC:%.cpp=$(BUILD_DIR)/%.o)
HEADERS = $(HEADER_FILES:%.h=src/%.h)
LIBS = -L$(BIN_DIR)/ -L$(HALIDE_HOME)/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lz

# Example code
EXAMPLES_SRC_FILES = $(EXAMPLES_DIR)/example1.cpp $(EXAMPLES_DIR)/example2.cpp $(EXAMPLES_DIR)/example3.cpp $(EXAMPLES_DIR)/example4.cpp $(EXAMPLES_DIR)/scheduling_example.cpp
EXAMPLES_AOT_FILES = $(GEN_DIR)/halide_sched_example.o

USE_HALIDE_JIT    = 1
USE_HALIDE_AOT	  = 2 
GENERATE_AOT_OBJS = 3

$(BUILD_DIR)/kernels/%.o: kernels/%.cpp
	@-mkdir -p $(BUILD_DIR)/kernels
	$(CXX) $(CXX_FLAGS) -c $< -I. -I$(HALIDE_HOME)/include $(LIBS) -o $@

#$(BUILD_DIR)/examples/%.o: examples/%.cpp
#	@-mkdir -p $(BUILD_DIR)/examples
#	$(CXX) $(CXX_FLAGS) -c $< -I. -I$(HALIDE_HOME)/include $(LIBS) -o $@

$(BIN_DIR)/test: main.cpp $(EXAMPLES_SRC_FILES) $(BIN_DIR)/libExcursions.a
	$(CXX) $(CXX_FLAGS)  $< -DUSAGE=$(USE_HALIDE_JIT) $(EXAMPLES_SRC_FILES) -I. -I$(HALIDE_HOME)/include $(LIBS) -lExcursions -o $@

# This rule first generates the Halide AOT objects, then links with them and runs the tests
$(BIN_DIR)/test_aot: main.cpp $(EXAMPLES_SRC_FILES) $(BIN_DIR)/libExcursions.a $(BIN_DIR)/generate_aot
	$(CXX) $(CXX_FLAGS)  $< -DUSAGE=$(USE_HALIDE_AOT) $(EXAMPLES_SRC_FILES) -I. -I$(HALIDE_HOME)/include $(LIBS) -lExcursions $(EXAMPLES_AOT_FILES) -o $@

# This rule generates Ahead-of-Time (AoT) code (static compilation of Halide functions)
# The objects and headers are placed in $(GEN_DIR)
$(BIN_DIR)/generate_aot: $(EXAMPLES_SRC_FILES) $(BIN_DIR)/libExcursions.a
	$(CXX) $(CXX_FLAGS) $< -DUSAGE=$(GENERATE_AOT_OBJS) $(EXAMPLES_DIR)/scheduling_example.cpp -I. -I$(HALIDE_HOME)/include -I$(GEN_DIR) $(LIBS) -lExcursions -o $@
	@-mkdir -p $(GEN_DIR)
	@-cd $(GEN_DIR)
	LD_LIBRARY_PATH=$(HALIDE_HOME)/bin $(BIN_DIR)/generate_aot $(GEN_DIR)

# This is the Excursions library which contains the source code of various Halide functions
$(BIN_DIR)/libExcursions.a: $(FUNCS_OBJ) $(HEADER_FILES)
	$(LD) -r -o  $(BUILD_DIR)/Excursions.o $(FUNCS_OBJ)
	@-mkdir -p $(BIN_DIR)
	rm -f $(BIN_DIR)/libExcursions.a
	ar q $(BIN_DIR)/libExcursions.a $(BUILD_DIR)/Excursions.o
	ranlib $(BIN_DIR)/libExcursions.a

.PHONY: all
all:  $(BIN_DIR)/test $(BIN_DIR)/test_aot

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*
	rm -rf $(BIN_DIR)/*
	rm -rf $(GEN_DIR)/*

#  ./halide_compile
# LD_LIBRARY_PATH=$HALIDE_HOME/bin ./halide_compile

# make
# LD_LIBRARY_PATH=$HALIDE_HOME/bin bin/test sched && cat dump.txt