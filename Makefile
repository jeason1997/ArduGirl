CXX ?= g++
BUILD_DIR := build
TARGET := $(BUILD_DIR)/ardugirl-terminal
SMOKE_TARGET := $(BUILD_DIR)/terminal-smoke
TEST_TARGET := $(BUILD_DIR)/framebuffer-test

CPPFLAGS := -Iinclude -I$(BUILD_DIR)/generated
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
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	games/examples/arduboy2_hello/entry.cpp

MICROTD_TARGET := $(BUILD_DIR)/microtd-terminal
MICROTD_SOURCES := \
	$(COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	games/ports/microtd/entry.cpp

SMOKE_SOURCES := \
	$(COMMON_SOURCES) \
	tests/smoke/terminal_game.cpp

OBJECTS := $(ARDUBOY_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
MICROTD_OBJECTS := $(MICROTD_SOURCES:%.cpp=$(BUILD_DIR)/microtd/%.o)
SMOKE_OBJECTS := $(SMOKE_SOURCES:%.cpp=$(BUILD_DIR)/smoke/%.o)
DEPENDS := $(OBJECTS:.o=.d)
TEST_OBJECTS := \
	$(BUILD_DIR)/src/core/framebuffer.o \
	$(BUILD_DIR)/tests/framebuffer_test.o

.PHONY: all run demo microtd test smoke clean check-upstream

all: check-upstream $(TARGET)

check-upstream:
	@test -f third_party/Arduboy2/examples/HelloWorld/HelloWorld.ino || \
		(printf '%s\n' '缺少 Arduboy2 子模块，请运行：git submodule update --init --recursive' && false)
	@test -f third_party/MicroTD/microtd.ino || \
		(printf '%s\n' '缺少 MicroTD 子模块，请运行：git submodule update --init --recursive' && false)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(SMOKE_TARGET): $(SMOKE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SMOKE_OBJECTS) $(LDFLAGS) -o $@

$(MICROTD_TARGET): $(MICROTD_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(MICROTD_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/smoke/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/microtd/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/generated/font5x7.inc: third_party/Arduboy2/src/Arduboy2Data.cpp
	@mkdir -p $(@D)
	@sed -n '/font5x7\[\] = {/,/^};/p' $< | sed '1d;$$d' > $@

$(BUILD_DIR)/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/microtd/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

run: $(TARGET)
	$(TARGET)

microtd: check-upstream $(MICROTD_TARGET)
	$(MICROTD_TARGET)

# 限定帧数的演示适合日志展示，并且不要求连接交互式终端。
demo: $(TARGET)
	$(TARGET) --frames 1 --plain

$(TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TEST_OBJECTS) $(LDFLAGS) -o $@

test: $(TARGET) $(MICROTD_TARGET) $(TEST_TARGET)
	$(TEST_TARGET)
	@output="$$($(TARGET) --frames 1 --plain)"; \
	printf '%s\n' "$$output"; \
	printf '%s' "$$output" | grep -q "ArduGirl terminal"
	@microtd_output="$$($(MICROTD_TARGET) --frames 1 --plain)"; \
	printf '%s' "$$microtd_output" | grep -q "ArduGirl terminal | MicroTD"

smoke: $(SMOKE_TARGET)
	$(SMOKE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPENDS) $(MICROTD_OBJECTS:.o=.d) $(SMOKE_OBJECTS:.o=.d) $(BUILD_DIR)/tests/framebuffer_test.d
