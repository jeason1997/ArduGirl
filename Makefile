CXX ?= g++
BUILD_DIR := build
TARGET := $(BUILD_DIR)/ardugirl-sdl
TERMINAL_TARGET := $(BUILD_DIR)/ardugirl-terminal
SMOKE_TARGET := $(BUILD_DIR)/terminal-smoke
TEST_TARGET := $(BUILD_DIR)/framebuffer-test
SDL_TEST_TARGET := $(BUILD_DIR)/sdl-backend-test
STORAGE_TEST_TARGET := $(BUILD_DIR)/storage-test
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)

CPPFLAGS := -Iinclude -I$(BUILD_DIR)/generated
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS ?=

RUNTIME_SOURCES := \
	src/core/framebuffer.cpp \
	src/runtime/main.cpp

LINUX_STORAGE_SOURCE := platform/linux_common/storage.cpp
SDL_COMMON_SOURCES := $(RUNTIME_SOURCES) platform/linux_sdl/sdl.cpp platform/linux_sdl/render.cpp $(LINUX_STORAGE_SOURCE)
TERMINAL_COMMON_SOURCES := $(RUNTIME_SOURCES) platform/linux_terminal/terminal.cpp $(LINUX_STORAGE_SOURCE)

DEFAULT_GAME_SOURCE :=
PORT_TEST_TARGETS :=
PORT_TERMINAL_TEST_TARGETS :=
PORT_DEPENDS :=
GAME_MAKEFILES := $(wildcard games/examples/*/port.mk games/ports/*/port.mk)
include $(GAME_MAKEFILES)

ARDUBOY_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	$(DEFAULT_GAME_SOURCE)

TERMINAL_ARDUBOY_SOURCES := $(filter-out platform/linux_sdl/render.cpp platform/linux_common/storage.cpp,\
	$(ARDUBOY_SOURCES:platform/linux_sdl/sdl.cpp=platform/linux_terminal/terminal.cpp))
TERMINAL_ARDUBOY_SOURCES += $(LINUX_STORAGE_SOURCE)
SMOKE_SOURCES := \
	$(TERMINAL_COMMON_SOURCES) \
	tests/smoke/terminal_game.cpp

OBJECTS := $(ARDUBOY_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TERMINAL_OBJECTS := $(TERMINAL_ARDUBOY_SOURCES:%.cpp=$(BUILD_DIR)/terminal/%.o)
SMOKE_OBJECTS := $(SMOKE_SOURCES:%.cpp=$(BUILD_DIR)/smoke/%.o)
DEPENDS := $(OBJECTS:.o=.d)
TEST_OBJECTS := \
	$(BUILD_DIR)/src/core/framebuffer.o \
	$(BUILD_DIR)/tests/framebuffer_test.o
SDL_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/sdl_backend_test.o \
	$(BUILD_DIR)/platform/linux_sdl/sdl.o \
	$(BUILD_DIR)/platform/linux_sdl/render.o \
	$(BUILD_DIR)/platform/linux_common/storage.o \
	$(BUILD_DIR)/src/core/framebuffer.o
STORAGE_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/storage_test.o \
	$(BUILD_DIR)/platform/linux_common/storage.o

.PHONY: all run demo terminal run-terminal test test-terminal smoke clean check-upstream check-sdl

all: check-upstream check-sdl $(TARGET)

check-upstream:
	@test -f third_party/Arduboy2/src/Arduboy2.h || \
		(printf '%s\n' '缺少 Arduboy2 子模块，请运行：git submodule update --init --recursive' && false)

check-sdl:
	@pkg-config --exists sdl2 || \
		(printf '%s\n' '缺少 SDL2 开发包或 pkg-config，无法构建默认 SDL 后端' && false)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(SDL_LIBS) -o $@

$(TERMINAL_TARGET): $(TERMINAL_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TERMINAL_OBJECTS) $(LDFLAGS) -o $@

$(SMOKE_TARGET): $(SMOKE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SMOKE_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/terminal/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/smoke/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/generated/font5x7.inc: third_party/Arduboy2/src/Arduboy2Data.cpp
	@mkdir -p $(@D)
	@sed -n '/font5x7\[\] = {/,/^};/p' $< | sed '1d;$$d' > $@

$(BUILD_DIR)/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/terminal/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

run: check-sdl $(TARGET)
	$(TARGET)

terminal: check-upstream $(TERMINAL_TARGET)

run-terminal: $(TERMINAL_TARGET)
	$(TERMINAL_TARGET)

# SDL 演示运行固定帧数后自动退出，便于开发时快速检查窗口显示。
demo: check-sdl $(TARGET)
	$(TARGET) --frames 180

$(TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TEST_OBJECTS) $(LDFLAGS) -o $@

$(SDL_TEST_TARGET): $(SDL_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SDL_TEST_OBJECTS) $(LDFLAGS) $(SDL_LIBS) -o $@

$(STORAGE_TEST_TARGET): $(STORAGE_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(STORAGE_TEST_OBJECTS) $(LDFLAGS) -o $@

test: check-sdl $(TARGET) $(TEST_TARGET) $(SDL_TEST_TARGET) $(STORAGE_TEST_TARGET) $(PORT_TEST_TARGETS)
	$(TEST_TARGET)
	$(SDL_TEST_TARGET)
	$(STORAGE_TEST_TARGET)
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	$(TARGET) --headless --frames 1 --save-dir "$$save_dir"

test-terminal: $(TERMINAL_TARGET) $(PORT_TERMINAL_TEST_TARGETS)
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	output="$$($(TERMINAL_TARGET) --frames 1 --plain --save-dir "$$save_dir")"; \
	printf '%s' "$$output" | grep -q "ArduGirl terminal"; \
	printf '%s' "$$output" | grep -q "ArduGirl terminal"

smoke: $(SMOKE_TARGET)
	$(SMOKE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPENDS) $(TERMINAL_OBJECTS:.o=.d) $(SMOKE_OBJECTS:.o=.d) \
	$(BUILD_DIR)/tests/framebuffer_test.d \
	$(PORT_DEPENDS)
