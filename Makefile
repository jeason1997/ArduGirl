CXX ?= g++
.DEFAULT_GOAL := all
BUILD_DIR := build
SMOKE_TARGET := $(BUILD_DIR)/terminal-smoke
TEST_TARGET := $(BUILD_DIR)/framebuffer-test
SDL_TEST_TARGET := $(BUILD_DIR)/sdl-backend-test
STORAGE_TEST_TARGET := $(BUILD_DIR)/storage-test
COMPAT_TEST_TARGET := $(BUILD_DIR)/compat-test
PLAYTUNE_TEST_TARGET := $(BUILD_DIR)/playtune-test
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)

CPPFLAGS := -Iinclude -I$(BUILD_DIR)/generated
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS ?=

RUNTIME_SOURCES := \
	src/core/framebuffer.cpp \
	src/compat/ArduboyPlaytune.cpp \
	src/runtime/main.cpp

LINUX_STORAGE_SOURCE := platform/linux/storage.cpp
SDL_COMMON_SOURCES := $(RUNTIME_SOURCES) platform/linux/sdl.cpp platform/linux/render.cpp $(LINUX_STORAGE_SOURCE)
TERMINAL_COMMON_SOURCES := $(RUNTIME_SOURCES) platform/linux/terminal.cpp $(LINUX_STORAGE_SOURCE)

PORT_BUILD_TARGETS :=
PORT_TEST_TARGETS :=
PORT_TERMINAL_TEST_TARGETS :=
PORT_DEPENDS :=
GAME_MAKEFILES := $(wildcard games/*/port.mk)
include $(GAME_MAKEFILES)
SMOKE_SOURCES := \
	$(TERMINAL_COMMON_SOURCES) \
	tests/smoke/terminal_game.cpp

SMOKE_OBJECTS := $(SMOKE_SOURCES:%.cpp=$(BUILD_DIR)/smoke/%.o)
TEST_OBJECTS := \
	$(BUILD_DIR)/src/core/framebuffer.o \
	$(BUILD_DIR)/tests/framebuffer_test.o
SDL_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/sdl_backend_test.o \
	$(BUILD_DIR)/platform/linux/sdl.o \
	$(BUILD_DIR)/platform/linux/render.o \
	$(BUILD_DIR)/platform/linux/storage.o \
	$(BUILD_DIR)/src/core/framebuffer.o
STORAGE_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/storage_test.o \
	$(BUILD_DIR)/platform/linux/storage.o
COMPAT_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/compat_test.o \
	$(BUILD_DIR)/src/core/framebuffer.o \
	$(BUILD_DIR)/src/arduboy2/Arduboy2.o \
	$(BUILD_DIR)/src/compat/ArduboyPlaytune.o \
	$(BUILD_DIR)/src/arduboy2/Sprites.o \
	$(BUILD_DIR)/src/compat/EEPROM.o
PLAYTUNE_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/playtune_test.o \
	$(BUILD_DIR)/src/compat/ArduboyPlaytune.o

.PHONY: all test test-terminal smoke clean check-upstream check-sdl

all: check-upstream check-sdl $(PORT_BUILD_TARGETS)

check-upstream:
	@test -f third_party/Arduboy2/src/Arduboy2.h || \
		(printf '%s\n' '缺少 Arduboy2 子模块，请运行：git submodule update --init --recursive' && false)

check-sdl:
	@pkg-config --exists sdl2 || \
		(printf '%s\n' '缺少 SDL2 开发包或 pkg-config，无法构建默认 SDL 后端' && false)

$(SMOKE_TARGET): $(SMOKE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SMOKE_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/smoke/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/generated/font5x7.inc: third_party/Arduboy2/src/Arduboy2Data.cpp
	@mkdir -p $(@D)
	@sed -n '/font5x7\[\] = {/,/^};/p' $< | sed '1d;$$d' > $@

$(BUILD_DIR)/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

$(TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TEST_OBJECTS) $(LDFLAGS) -o $@

$(SDL_TEST_TARGET): $(SDL_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SDL_TEST_OBJECTS) $(LDFLAGS) $(SDL_LIBS) -o $@

$(STORAGE_TEST_TARGET): $(STORAGE_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(STORAGE_TEST_OBJECTS) $(LDFLAGS) -o $@

$(COMPAT_TEST_TARGET): $(COMPAT_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(COMPAT_TEST_OBJECTS) $(LDFLAGS) -o $@

$(PLAYTUNE_TEST_TARGET): $(PLAYTUNE_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(PLAYTUNE_TEST_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

test: check-sdl $(TEST_TARGET) $(SDL_TEST_TARGET) $(STORAGE_TEST_TARGET) $(COMPAT_TEST_TARGET) $(PLAYTUNE_TEST_TARGET) $(PORT_TEST_TARGETS)
	$(TEST_TARGET)
	$(SDL_TEST_TARGET)
	$(STORAGE_TEST_TARGET)
	$(COMPAT_TEST_TARGET)
	$(PLAYTUNE_TEST_TARGET)

test-terminal: $(PORT_TERMINAL_TEST_TARGETS)

smoke: $(SMOKE_TARGET)
	$(SMOKE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(SMOKE_OBJECTS:.o=.d) \
	$(BUILD_DIR)/tests/framebuffer_test.d \
	$(PORT_DEPENDS)
