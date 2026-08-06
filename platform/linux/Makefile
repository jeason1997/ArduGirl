CXX ?= g++
.DEFAULT_GOAL := all
BUILD_DIR := build
SMOKE_TARGET := $(BUILD_DIR)/terminal-smoke
TEST_TARGET := $(BUILD_DIR)/framebuffer-test
SDL_TEST_TARGET := $(BUILD_DIR)/sdl-backend-test
STORAGE_TEST_TARGET := $(BUILD_DIR)/storage-test
COMPAT_TEST_TARGET := $(BUILD_DIR)/compat-test
PLAYTUNE_TEST_TARGET := $(BUILD_DIR)/playtune-test
ATM_TEST_TARGET := $(BUILD_DIR)/atm-test
STDIO_TEST_TARGET := $(BUILD_DIR)/stdio-test
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)

CPPFLAGS := -Iinclude -I$(BUILD_DIR)/generated -Iplatform/linux/include
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LINUX_RUNTIME_LDFLAGS := $(LDFLAGS) -rdynamic

RUNTIME_SOURCES := \
	src/core/framebuffer.cpp \
	src/compat/ATMlib.cpp \
	src/compat/ArduboyPlaytune.cpp \
	src/runtime/main.cpp

LINUX_STORAGE_SOURCE := platform/linux/src/storage.cpp
LINUX_CRASH_SOURCE := platform/linux/src/crash.cpp
SDL_COMMON_SOURCES := $(RUNTIME_SOURCES) platform/linux/src/sdl.cpp platform/linux/src/render.cpp $(LINUX_STORAGE_SOURCE) $(LINUX_CRASH_SOURCE)
TERMINAL_COMMON_SOURCES := $(RUNTIME_SOURCES) platform/linux/src/terminal.cpp $(LINUX_STORAGE_SOURCE) $(LINUX_CRASH_SOURCE)

PORT_BUILD_TARGETS :=
PORT_TEST_TARGETS :=
PORT_TERMINAL_TEST_TARGETS :=
PORT_DEPENDS :=
ifneq ($(strip $(GAME)),)
DIRECT_GAME_MAKEFILE := $(wildcard games/$(GAME)/port.mk)
COLLECTION_GAME_MANIFESTS := $(wildcard games/*/$(GAME)/game.toml)
COLLECTION_GAME_MAKEFILES := $(foreach manifest,$(COLLECTION_GAME_MANIFESTS),$(dir $(patsubst %/,%,$(dir $(manifest))))port.mk)
GAME_MAKEFILES := $(sort $(DIRECT_GAME_MAKEFILE) $(COLLECTION_GAME_MAKEFILES))
ifeq ($(strip $(GAME_MAKEFILES)),)
$(error 未找到 Linux 游戏：$(GAME))
endif
else
GAME_MAKEFILES := $(wildcard games/*/port.mk)
endif
include $(GAME_MAKEFILES)

ifneq ($(strip $(GAME)),)
SELECTED_GAME_TARGETS := $(filter $(BUILD_DIR)/%-$(GAME)-sdl $(BUILD_DIR)/$(GAME)-sdl,$(PORT_BUILD_TARGETS))
ifeq ($(strip $(SELECTED_GAME_TARGETS)),)
$(error 未找到 Linux 游戏：$(GAME))
endif
ifneq ($(words $(SELECTED_GAME_TARGETS)),1)
$(error Linux 游戏 $(GAME) 匹配到多个构建目标：$(SELECTED_GAME_TARGETS))
endif
endif

ifneq ($(filter build run,$(MAKECMDGOALS)),)
ifeq ($(strip $(GAME)),)
$(error build 和 run 目标必须通过 GAME=<game-id> 选择游戏)
endif
endif

SMOKE_SOURCES := \
	$(TERMINAL_COMMON_SOURCES) \
	platform/linux/tests/terminal_game.cpp

SMOKE_OBJECTS := $(SMOKE_SOURCES:%.cpp=$(BUILD_DIR)/smoke/%.o)
TEST_OBJECTS := \
	$(BUILD_DIR)/src/core/framebuffer.o \
	$(BUILD_DIR)/tests/framebuffer_test.o
SDL_TEST_OBJECTS := \
	$(BUILD_DIR)/platform/linux/tests/sdl_backend_test.o \
	$(BUILD_DIR)/platform/linux/src/sdl.o \
	$(BUILD_DIR)/platform/linux/src/crash.o \
	$(BUILD_DIR)/platform/linux/src/render.o \
	$(BUILD_DIR)/platform/linux/src/storage.o \
	$(BUILD_DIR)/src/core/framebuffer.o
STORAGE_TEST_OBJECTS := \
	$(BUILD_DIR)/platform/linux/tests/storage_test.o \
	$(BUILD_DIR)/platform/linux/src/storage.o
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
ATM_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/atm_test.o \
	$(BUILD_DIR)/src/compat/ATMlib.o
STDIO_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/stdio_test.o \
	$(BUILD_DIR)/src/compat/stdio.o

.PHONY: all build run test test-terminal smoke clean check-upstream check-sdl

all: check-upstream check-sdl $(if $(strip $(GAME)),$(SELECTED_GAME_TARGETS),$(PORT_BUILD_TARGETS))
	@if [ -n "$(strip $(GAME))" ]; then $(SELECTED_GAME_TARGETS); fi

build: check-upstream check-sdl $(SELECTED_GAME_TARGETS)
	@printf '%s\n' 'SDL 程序已生成：$(SELECTED_GAME_TARGETS)'

run: check-upstream check-sdl $(SELECTED_GAME_TARGETS)
	$(SELECTED_GAME_TARGETS)

check-upstream:
	@test -f third_party/Arduboy2/src/Arduboy2.h || \
		(printf '%s\n' '缺少 Arduboy2 子模块，请运行：git submodule update --init --recursive' && false)

check-sdl:
	@pkg-config --exists sdl2 || \
		(printf '%s\n' '缺少 SDL2 开发包或 pkg-config，无法构建默认 SDL 后端' && false)

$(SMOKE_TARGET): $(SMOKE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SMOKE_OBJECTS) $(LINUX_RUNTIME_LDFLAGS) -o $@

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

$(ATM_TEST_TARGET): $(ATM_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(ATM_TEST_OBJECTS) $(LDFLAGS) -o $@

$(STDIO_TEST_OBJECTS): CXXFLAGS += -fno-builtin-sprintf

$(STDIO_TEST_TARGET): $(STDIO_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(STDIO_TEST_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

test: check-sdl $(TEST_TARGET) $(SDL_TEST_TARGET) $(STORAGE_TEST_TARGET) $(COMPAT_TEST_TARGET) $(PLAYTUNE_TEST_TARGET) $(ATM_TEST_TARGET) $(STDIO_TEST_TARGET) $(PORT_TEST_TARGETS)
	$(TEST_TARGET)
	$(SDL_TEST_TARGET)
	$(STORAGE_TEST_TARGET)
	$(COMPAT_TEST_TARGET)
	$(PLAYTUNE_TEST_TARGET)
	$(ATM_TEST_TARGET)
	$(STDIO_TEST_TARGET)

test-terminal: $(PORT_TERMINAL_TEST_TARGETS)

smoke: $(SMOKE_TARGET)
	$(SMOKE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(SMOKE_OBJECTS:.o=.d) \
	$(BUILD_DIR)/tests/framebuffer_test.d \
	$(if $(PORT_DEPENDS_OVERRIDE),$(PORT_DEPENDS_OVERRIDE),$(PORT_DEPENDS))
