CXX ?= g++
BUILD_DIR := build
TARGET := $(BUILD_DIR)/ardugirl-terminal
SMOKE_TARGET := $(BUILD_DIR)/terminal-smoke
TEST_TARGET := $(BUILD_DIR)/framebuffer-test

CPPFLAGS := -Iinclude
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS ?=

COMMON_SOURCES := \
	src/core/framebuffer.cpp \
	src/runtime/main.cpp \
	platform/linux_terminal/terminal.cpp

ARDUBOY_SOURCES := \
	$(COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	games/examples/arduboy2_hello/entry.cpp

SMOKE_SOURCES := \
	$(COMMON_SOURCES) \
	tests/smoke/terminal_game.cpp

OBJECTS := $(ARDUBOY_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
SMOKE_OBJECTS := $(SMOKE_SOURCES:%.cpp=$(BUILD_DIR)/smoke/%.o)
DEPENDS := $(OBJECTS:.o=.d)
TEST_OBJECTS := \
	$(BUILD_DIR)/src/core/framebuffer.o \
	$(BUILD_DIR)/tests/framebuffer_test.o

.PHONY: all run demo test smoke clean check-upstream

all: check-upstream $(TARGET)

check-upstream:
	@test -f third_party/Arduboy2/examples/HelloWorld/HelloWorld.ino || \
		(printf '%s\n' '缺少 Arduboy2 子模块，请运行：git submodule update --init --recursive' && false)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(SMOKE_TARGET): $(SMOKE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SMOKE_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/smoke/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	$(TARGET)

# 限定帧数的演示适合日志展示，并且不要求连接交互式终端。
demo: $(TARGET)
	$(TARGET) --frames 1 --plain

$(TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TEST_OBJECTS) $(LDFLAGS) -o $@

test: $(TARGET) $(TEST_TARGET)
	$(TEST_TARGET)
	@output="$$($(TARGET) --frames 1 --plain)"; \
	printf '%s\n' "$$output"; \
	printf '%s' "$$output" | grep -q "ArduGirl terminal"

smoke: $(SMOKE_TARGET)
	$(SMOKE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPENDS) $(SMOKE_OBJECTS:.o=.d) $(BUILD_DIR)/tests/framebuffer_test.d
