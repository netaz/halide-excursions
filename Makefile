# Before running make you must set HALIDE_HOME to the directory where the Halide source code resides
HALIDE_HOME ?= HALIDE_HOME_MUST_BE_SET
GTEST_HOME ?= GTEST_HOME_MUST_BE_SET

CXX ?= g++
OPTIMIZE ?= -O3
CXX11 ?=

CXX_WARNING_FLAGS = -Wall -Werror -Wno-unused-function -Wcast-qual
CXX_FLAGS = $(CXX_WARNING_FLAGS) -fno-rtti -Woverloaded-virtual -fPIC $(OPTIMIZE) -fno-omit-frame-pointer
CXX_FLAGS += -std=c++0x

BUILD_DIR = build
BIN_DIR = bin
SAMPLES_DIR = samples
TESTS_DIR = tests
GEN_DIR = generated
FUNCS_DIR = functions
FUNCS_SRC = $(FUNCS_DIR)/cv.cpp $(FUNCS_DIR)/color_convert.cpp $(FUNCS_DIR)/openvx.cpp
EXCUR_HEADER_FILES = ./utils/clock.h ./utils/utils.h

FUNCS_OBJ = $(FUNCS_SRC:%.cpp=$(BUILD_DIR)/%.o)
#HEADERS = $(HEADER_FILES:%.h=src/%.h)
LIBS = -L$(BIN_DIR)/ -L$(HALIDE_HOME)/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lz
LIBS += -L$(GTEST_HOME)/lib/.libs -lgtest
HEADERS = -I. -I$(HALIDE_HOME)/include 
HEADERS += -I$(GTEST_HOME)/include

# Unit tests
TESTS_SRC_FILES = $(TESTS_DIR)/box3x3_test.cpp 

# Example code
SAMPLES_SRC_FILES = $(SAMPLES_DIR)/sample1.cpp $(SAMPLES_DIR)/sample2.cpp \
					$(SAMPLES_DIR)/sample3.cpp $(SAMPLES_DIR)/sample4.cpp $(SAMPLES_DIR)/scheduling_sample.cpp
SAMPLES_AOT_FILES = $(GEN_DIR)/halide_sched_example.o

USE_HALIDE_JIT    = 1
USE_HALIDE_AOT	  = 2 
GENERATE_AOT_OBJS = 3

$(BUILD_DIR)/$(FUNCS_DIR)/%.o: $(FUNCS_DIR)/%.cpp 
	@-mkdir -p $(BUILD_DIR)/$(FUNCS_DIR)
	$(CXX) $(CXX_FLAGS) -c $< -I. -I$(HALIDE_HOME)/include $(LIBS) -o $@

#$(BUILD_DIR)/samples/%.o: samples/%.cpp
#	@-mkdir -p $(BUILD_DIR)/samples
#	$(CXX) $(CXX_FLAGS) -c $< -I. -I$(HALIDE_HOME)/include $(LIBS) -o $@

$(BIN_DIR)/test: main.cpp $(SAMPLES_SRC_FILES) $(BIN_DIR)/libExcursions.a
	$(CXX) $(CXX_FLAGS)  $< -DUSAGE=$(USE_HALIDE_JIT) $(SAMPLES_SRC_FILES) $(HEADERS) $(LIBS) -lExcursions -o $@

# This rule first generates the Halide AOT objects, then links with them and runs the tests
$(BIN_DIR)/test_aot: main.cpp $(SAMPLES_SRC_FILES) $(BIN_DIR)/libExcursions.a $(BIN_DIR)/generate_aot
	$(CXX) $(CXX_FLAGS)  $< -DUSAGE=$(USE_HALIDE_AOT) $(SAMPLES_SRC_FILES) $(HEADERS) $(LIBS) -lExcursions $(SAMPLES_AOT_FILES) -o $@

unit_tests: $(BIN_DIR)/unit_tests

$(BIN_DIR)/unit_tests: $(TESTS_SRC_FILES) $(BIN_DIR)/libExcursions.a
	$(CXX) $(CXX_FLAGS)  $< -DUSAGE=$(USE_HALIDE_JIT) $(HEADERS) $(LIBS) -lExcursions -o $@
	LD_LIBRARY_PATH=$(HALIDE_HOME)/bin:$(GTEST_HOME)/lib/.libs ./bin/unit_tests

# This rule generates Ahead-of-Time (AoT) code (static compilation of Halide functions)
# The objects and headers are placed in $(GEN_DIR)
$(BIN_DIR)/generate_aot: $(SAMPLES_SRC_FILES) $(BIN_DIR)/libExcursions.a
	$(CXX) $(CXX_FLAGS) $< -DUSAGE=$(GENERATE_AOT_OBJS) $(SAMPLES_DIR)/scheduling_sample.cpp $(HEADERS) -I$(GEN_DIR) $(LIBS) -lExcursions -o $@
	@-mkdir -p $(GEN_DIR)
	@-cd $(GEN_DIR)
	LD_LIBRARY_PATH=$(HALIDE_HOME)/bin $(BIN_DIR)/generate_aot $(GEN_DIR)

# This is the Excursions library which contains the source code of various Halide functions
$(BIN_DIR)/libExcursions.a: $(FUNCS_OBJ) $(EXCUR_HEADER_FILES) 
	$(LD) -r -o  $(BUILD_DIR)/Excursions.o $(FUNCS_OBJ)
	@-mkdir -p $(BIN_DIR)
	rm -f $(BIN_DIR)/libExcursions.a
	ar q $(BIN_DIR)/libExcursions.a $(BUILD_DIR)/Excursions.o
	ranlib $(BIN_DIR)/libExcursions.a

.PHONY: all
all:  $(BIN_DIR)/test $(BIN_DIR)/test_aot $(BIN_DIR)/unit_tests

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*
	rm -rf $(BIN_DIR)/*
	rm -rf $(GEN_DIR)/*

#  ./halide_compile
# LD_LIBRARY_PATH=$HALIDE_HOME/bin ./halide_compile

# make
# LD_LIBRARY_PATH=$HALIDE_HOME/bin bin/test sched && cat dump.txt