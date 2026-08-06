CXX ?= g++
BUILD_DIR := build
TARGET := $(BUILD_DIR)/ardugirl-sdl
TERMINAL_TARGET := $(BUILD_DIR)/ardugirl-terminal
SMOKE_TARGET := $(BUILD_DIR)/terminal-smoke
TEST_TARGET := $(BUILD_DIR)/framebuffer-test
SDL_TEST_TARGET := $(BUILD_DIR)/sdl-backend-test
MICROTD_REPLAY_TEST_TARGET := $(BUILD_DIR)/microtd-replay-test
STORAGE_TEST_TARGET := $(BUILD_DIR)/storage-test
ARDUBOYWORKS_GAMES := hollow hopper chribocchi chiemagari psicolo reversi lasers quarto stairssweep pi24k samegame knightmove ardubullets evasion morse gosencho bananonsense toyokumono
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

ARDUBOY_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	games/examples/arduboy2_hello/entry.cpp

TERMINAL_ARDUBOY_SOURCES := $(filter-out platform/linux_sdl/render.cpp platform/linux_common/storage.cpp,\
	$(ARDUBOY_SOURCES:platform/linux_sdl/sdl.cpp=platform/linux_terminal/terminal.cpp))
TERMINAL_ARDUBOY_SOURCES += $(LINUX_STORAGE_SOURCE)
MICROTD_TARGET := $(BUILD_DIR)/microtd-sdl
TERMINAL_MICROTD_TARGET := $(BUILD_DIR)/microtd-terminal
MICROTD_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	games/ports/microtd/entry.cpp

define ARDUBOYWORKS_GAME_template
ARDUBOYWORKS_$(1)_SOURCES := \
	$$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	$$(filter-out third_party/ArduboyWorks/$(1)/MyArduboyPlaytune.cpp,$$(wildcard third_party/ArduboyWorks/$(1)/*.cpp)) \
	games/ports/arduboyworks/entry.cpp
ARDUBOYWORKS_$(1)_OBJECTS := $$(ARDUBOYWORKS_$(1)_SOURCES:%.cpp=$$(BUILD_DIR)/arduboyworks/$(1)/%.o)

$$(BUILD_DIR)/arduboyworks/$(1)/%.o: %.cpp
	@mkdir -p $$(@D)
	$$(CXX) $$(CPPFLAGS) $$(SDL_CFLAGS) $$(CXXFLAGS) -fpermissive -Wno-narrowing -DARDUINO=10819 -DUSE_ARDUBOY2_LIB \
		-Ithird_party/ArduboyWorks -Ithird_party/ArduboyWorks/$(1) -DARDUBOYWORKS_GAME_ID=$(1) \
		-DARDUBOYWORKS_ENTRY=\"$(1)/$(1).ino\" -MMD -MP -c $$< -o $$@

$$(BUILD_DIR)/arduboyworks-$(1)-sdl: $$(ARDUBOYWORKS_$(1)_OBJECTS)
	@mkdir -p $$(@D)
	$$(CXX) $$^ $$(LDFLAGS) $$(SDL_LIBS) -o $$@

.PHONY: $(1)
$(1): check-upstream check-sdl $$(BUILD_DIR)/arduboyworks-$(1)-sdl
	$$(BUILD_DIR)/arduboyworks-$(1)-sdl
endef

$(foreach game,$(ARDUBOYWORKS_GAMES),$(eval $(call ARDUBOYWORKS_GAME_template,$(game))))

TERMINAL_MICROTD_SOURCES := $(filter-out platform/linux_sdl/render.cpp platform/linux_common/storage.cpp,\
	$(MICROTD_SOURCES:platform/linux_sdl/sdl.cpp=platform/linux_terminal/terminal.cpp))
TERMINAL_MICROTD_SOURCES += $(LINUX_STORAGE_SOURCE)
SMOKE_SOURCES := \
	$(TERMINAL_COMMON_SOURCES) \
	tests/smoke/terminal_game.cpp

OBJECTS := $(ARDUBOY_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TERMINAL_OBJECTS := $(TERMINAL_ARDUBOY_SOURCES:%.cpp=$(BUILD_DIR)/terminal/%.o)
MICROTD_OBJECTS := $(MICROTD_SOURCES:%.cpp=$(BUILD_DIR)/microtd/%.o)
TERMINAL_MICROTD_OBJECTS := $(TERMINAL_MICROTD_SOURCES:%.cpp=$(BUILD_DIR)/terminal-microtd/%.o)
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
MICROTD_REPLAY_TEST_SOURCES := \
	tests/microtd_replay_test.cpp \
	src/core/framebuffer.cpp \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp
MICROTD_REPLAY_TEST_OBJECTS := $(MICROTD_REPLAY_TEST_SOURCES:%.cpp=$(BUILD_DIR)/replay/%.o)
STORAGE_TEST_OBJECTS := \
	$(BUILD_DIR)/tests/storage_test.o \
	$(BUILD_DIR)/platform/linux_common/storage.o

.PHONY: all run demo microtd terminal run-terminal microtd-terminal test test-terminal smoke clean check-upstream check-sdl

all: check-upstream check-sdl $(TARGET)

check-upstream:
	@test -f third_party/Arduboy2/examples/HelloWorld/HelloWorld.ino || \
		(printf '%s\n' '缺少 Arduboy2 子模块，请运行：git submodule update --init --recursive' && false)
	@test -f third_party/MicroTD/microtd.ino || \
		(printf '%s\n' '缺少 MicroTD 子模块，请运行：git submodule update --init --recursive' && false)

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

$(MICROTD_TARGET): $(MICROTD_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(MICROTD_OBJECTS) $(LDFLAGS) $(SDL_LIBS) -o $@

$(TERMINAL_MICROTD_TARGET): $(TERMINAL_MICROTD_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TERMINAL_MICROTD_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/terminal/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/smoke/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/microtd/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/terminal-microtd/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/replay/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/generated/font5x7.inc: third_party/Arduboy2/src/Arduboy2Data.cpp
	@mkdir -p $(@D)
	@sed -n '/font5x7\[\] = {/,/^};/p' $< | sed '1d;$$d' > $@

$(BUILD_DIR)/generated/microtd_patched.ino: third_party/MicroTD/microtd.ino \
		games/ports/microtd/patches/0001-build-selected-tower-return-true.patch
	@mkdir -p $(@D)
	@sed 's/\r$$//' $< > $@.base
	@patch --silent --output=$@ $@.base < games/ports/microtd/patches/0001-build-selected-tower-return-true.patch
	@rm -f $@.base

$(BUILD_DIR)/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/microtd/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/terminal/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/terminal-microtd/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/replay/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/microtd/games/ports/microtd/entry.o: $(BUILD_DIR)/generated/microtd_patched.ino
$(BUILD_DIR)/terminal-microtd/games/ports/microtd/entry.o: $(BUILD_DIR)/generated/microtd_patched.ino
$(BUILD_DIR)/replay/tests/microtd_replay_test.o: $(BUILD_DIR)/generated/microtd_patched.ino

run: check-sdl $(TARGET)
	$(TARGET)

microtd: check-upstream check-sdl $(MICROTD_TARGET)
	$(MICROTD_TARGET)

terminal: check-upstream $(TERMINAL_TARGET)

run-terminal: $(TERMINAL_TARGET)
	$(TERMINAL_TARGET)

microtd-terminal: check-upstream $(TERMINAL_MICROTD_TARGET)
	$(TERMINAL_MICROTD_TARGET)

# SDL 演示运行固定帧数后自动退出，便于开发时快速检查窗口显示。
demo: check-sdl $(TARGET)
	$(TARGET) --frames 180

$(TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TEST_OBJECTS) $(LDFLAGS) -o $@

$(SDL_TEST_TARGET): $(SDL_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(SDL_TEST_OBJECTS) $(LDFLAGS) $(SDL_LIBS) -o $@

$(MICROTD_REPLAY_TEST_TARGET): $(MICROTD_REPLAY_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(MICROTD_REPLAY_TEST_OBJECTS) $(LDFLAGS) -o $@

$(STORAGE_TEST_TARGET): $(STORAGE_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(STORAGE_TEST_OBJECTS) $(LDFLAGS) -o $@

test: check-sdl $(TARGET) $(MICROTD_TARGET) $(TEST_TARGET) $(SDL_TEST_TARGET) $(MICROTD_REPLAY_TEST_TARGET) $(STORAGE_TEST_TARGET)
	$(TEST_TARGET)
	$(SDL_TEST_TARGET)
	$(MICROTD_REPLAY_TEST_TARGET)
	$(STORAGE_TEST_TARGET)
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	$(TARGET) --headless --frames 1 --save-dir "$$save_dir"; \
	$(MICROTD_TARGET) --headless --frames 3 --save-dir "$$save_dir"

test-terminal: $(TERMINAL_TARGET) $(TERMINAL_MICROTD_TARGET)
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	output="$$($(TERMINAL_TARGET) --frames 1 --plain --save-dir "$$save_dir")"; \
	printf '%s' "$$output" | grep -q "ArduGirl terminal"; \
	microtd_output="$$($(TERMINAL_MICROTD_TARGET) --frames 1 --plain --save-dir "$$save_dir")"; \
	printf '%s' "$$microtd_output" | grep -q "ArduGirl terminal | MicroTD"; \
	arrow_lines="$$(printf '\033[A' | $(TERMINAL_MICROTD_TARGET) --frames 3 --plain --save-dir "$$save_dir" | wc -l)"; \
	test "$$arrow_lines" -ge 70

smoke: $(SMOKE_TARGET)
	$(SMOKE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPENDS) $(TERMINAL_OBJECTS:.o=.d) $(MICROTD_OBJECTS:.o=.d) \
	$(TERMINAL_MICROTD_OBJECTS:.o=.d) $(SMOKE_OBJECTS:.o=.d) \
	$(BUILD_DIR)/tests/framebuffer_test.d \
	$(MICROTD_REPLAY_TEST_OBJECTS:.o=.d)
