# ============================================================
#
#   The MIT License (MIT)
#
#   Copyright (c) 2012-2013 Michał Blinkiewicz <michal.blinkiewicz@gmail.com>
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy of
#   this software and associated documentation files (the "Software"), to deal in
#   the Software without restriction, including without limitation the rights to
#   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
#   the Software, and to permit persons to whom the Software is furnished to do so,
#   subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included in all
#   copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
#   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
#   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# ============================================================

# === main settings
# ============================================================

EXE_NAME := rcmd

COMPILER := g++
COMPILER_FLAGS := -Wall -std=c++11 #-DWITHOUT_VERBOSE

INCLUDE_DIRS := C:\MinGW\msys\1.0\mingw\include
LIBRARY_DIRS := C:\MinGW\msys\1.0\mingw\lib
LIBRARIES := ssh2 ssl crypto ws2_32 gdi32 z yaml-cpp

SRC_DIR := src
OBJ_DIR := obj
DEP_DIR := dep

# === magic :)
# ============================================================

SRC_DIRS := $(SRC_DIR)/ $(dir $(wildcard $(SRC_DIR)/*/.)) # only one level of subdirs

SRCS := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)*.cpp))
DEPS := $(patsubst $(SRC_DIR)/%,$(DEP_DIR)/%,$(SRCS:.cpp=.d))
OBJS := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SRCS:.cpp=.o))

$(info --- --------------------------------------------------)
$(info ---       making | $(EXE_NAME))
$(info --- --------------------------------------------------)
$(info ---     src dirs :)
$(foreach sdir,$(SRC_DIRS),$(info ---              - $(sdir)))
$(info ---      sources :)
$(foreach src,$(SRCS),$(info ---              - $(src)))
$(info --- dependencies :)
$(foreach dep,$(DEPS),$(info ---              - $(dep)))
$(info ---      objects :)
$(foreach obj,$(OBJS),$(info ---              - $(obj)))
$(info --- --------------------------------------------------)
$(info ---    Makefile (c) 2013 Michal Blinkiewicz)
$(info --- --------------------------------------------------)

CXX = $(COMPILER)
CXXFLAGS = $(addprefix -I,$(INCLUDE_DIRS)) $(COMPILER_FLAGS)
LDFLAGS = $(addprefix -L,$(LIBRARY_DIRS)) $(addprefix -l,$(LIBRARIES))

# === main part
# ============================================================

all: $(EXE_NAME)

$(EXE_NAME): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

include $(DEPS)

$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MF $@ -MG -MM -MP -MT $(OBJ_DIR)/$*.o -MT $@ $<

clean:
	-rm -rf $(EXE_NAME) $(OBJ_DIR) $(DEP_DIR)

.PHONY: all clean

# ============================================================

