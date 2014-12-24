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

FILTERS_DIR = $(BUILD_DIR)/filters
FUNCTIONS_SRC_FILES = kernels/openvx.cpp kernels/cv.cpp kernels/color_convert.cpp
EXAMPLES_SRC_FILES = $(EXAMPLES_DIR)/example1.cpp $(EXAMPLES_DIR)/example2.cpp $(EXAMPLES_DIR)/example3.cpp $(EXAMPLES_DIR)/example4.cpp
HEADER_FILES = 

OBJECTS = $(FUNCTIONS_SRC_FILES:%.cpp=$(BUILD_DIR)/%.o)
HEADERS = $(HEADER_FILES:%.h=src/%.h)
LIBS = -L$(BIN_DIR)/ -L$(HALIDE_HOME)/bin -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -lz

$(BUILD_DIR)/%.o: ./%.cpp ./%.h
	@-mkdir -p $(BUILD_DIR)/kernels
	$(CXX) $(CXX_FLAGS) -c $< -Ikernels -I$(HALIDE_HOME)/include $(LIBS) -o $(BUILD_DIR)/$*.o

$(BIN_DIR)/test: main.cpp $(EXAMPLES_SRC_FILES) $(BIN_DIR)/libExcursions.a
	$(CXX) $(CXX_FLAGS)  $< $(EXAMPLES_SRC_FILES) -I. -I$(HALIDE_HOME)/include $(LIBS) -lExcursions -o $@

$(BIN_DIR)/libExcursions.a: $(OBJECTS)
	$(LD) -r -o  $(BUILD_DIR)/Excursions.o $(OBJECTS)
	@-mkdir -p $(BIN_DIR)
	rm -f $(BIN_DIR)/libExcursions.a
	ar q $(BIN_DIR)/libExcursions.a $(BUILD_DIR)/Excursions.o
	ranlib $(BIN_DIR)/libExcursions.a

.PHONY: all
all:  $(BIN_DIR)/test

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*

