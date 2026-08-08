CXX ?= g++
CC ?= gcc
.DEFAULT_GOAL := all
BUILD_DIR := build
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
	src/compat/ArduboyTones.cpp \
	src/compat/ArduboyPlaytune.cpp \
	src/runtime/main.cpp

LINUX_STORAGE_SOURCE := platform/linux/src/storage.cpp
LINUX_CRASH_SOURCE := platform/linux/src/crash.cpp
SDL_COMMON_SOURCES := $(RUNTIME_SOURCES) platform/linux/src/sdl.cpp platform/linux/src/render.cpp $(LINUX_STORAGE_SOURCE) $(LINUX_CRASH_SOURCE)

GAME_PREPARER := tools/prepare_game.py
GAME_SUBMODULE_PREPARER := tools/ensure_game_submodules.py
GAME_CATALOG_GENERATOR := platform/linux/tools/generate_games_make.py
GAME_CATALOG_KEY := $(if $(strip $(GAME)),$(GAME),all)
GAME_CATALOG := $(BUILD_DIR)/linux/catalog-$(GAME_CATALOG_KEY).mk
ifneq ($(MAKECMDGOALS),clean)
GAME_SUBMODULE_STATUS := $(shell python $(GAME_SUBMODULE_PREPARER) $(if $(strip $(GAME)),--game $(GAME)); printf '%s' $$?)
ifneq ($(GAME_SUBMODULE_STATUS),0)
$(error 无法准备游戏子模块)
endif
GAME_CATALOG_STATUS := $(shell python $(GAME_CATALOG_GENERATOR) --output $(GAME_CATALOG) $(if $(strip $(GAME)),--game $(GAME)); printf '%s' $$?)
ifneq ($(GAME_CATALOG_STATUS),0)
$(error 无法生成 Linux 游戏目录)
endif
include $(GAME_CATALOG)
else
GAME_IDS :=
endif

PORT_BUILD_TARGETS :=
PORT_TEST_TARGETS :=
PORT_DEPENDS :=

GAME_COMPAT_SOURCES := \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp
GAME_REPLAY_COMMON_SOURCES := \
	src/core/framebuffer.cpp \
	src/compat/ATMlib.cpp \
	src/compat/ArduboyTones.cpp \
	src/compat/ArduboyPlaytune.cpp \
	$(GAME_COMPAT_SOURCES)

define GAME_template
GAME_SDL_TARGET_$(1) := $$(BUILD_DIR)/$(1)-sdl
GAME_SDL_SOURCES_$(1) := $$(SDL_COMMON_SOURCES) $$(GAME_COMPAT_SOURCES) \
	$$(GAME_ENTRY_CPP_$(1)) $$(GAME_CPP_SOURCES_$(1)) $$(GAME_EXTRA_CPP_SOURCES_$(1))
GAME_SDL_OBJECTS_$(1) := $$(GAME_SDL_SOURCES_$(1):%.cpp=$$(BUILD_DIR)/linux/$(1)/%.o) \
	$$(GAME_C_SOURCES_$(1):%.c=$$(BUILD_DIR)/linux/$(1)/%.o)
GAME_REPLAY_SOURCES_$(1) := $$(GAME_REPLAY_SOURCE_$(1)) $$(GAME_REPLAY_COMMON_SOURCES) \
	$$(GAME_CPP_SOURCES_$(1)) $$(GAME_EXTRA_CPP_SOURCES_$(1))
GAME_REPLAY_OBJECTS_$(1) := $$(GAME_REPLAY_SOURCES_$(1):%.cpp=$$(BUILD_DIR)/linux/replay-$(1)/%.o) \
	$$(GAME_C_SOURCES_$(1):%.c=$$(BUILD_DIR)/linux/replay-$(1)/%.o)

$$(GAME_GENERATED_ENTRY_$(1)): $$(GAME_SNAPSHOT_SOURCES_$(1)) $$(GAME_MANIFEST_$(1)) \
		$$(GAME_BUILD_CONFIG_$(1)) $$(GAME_PATCHES_$(1)) $$(GAME_PREPARER)
	python $$(GAME_PREPARER) $(1) --prepare

$$(GAME_ENTRY_CPP_$(1)): $$(GAME_GENERATED_ENTRY_$(1))
$$(GAME_CPP_SOURCES_$(1)) $$(GAME_C_SOURCES_$(1)): $$(GAME_GENERATED_ENTRY_$(1))

$$(BUILD_DIR)/linux/$(1)/%.o: %.cpp
	@mkdir -p $$(@D)
	$$(CXX) $$(CPPFLAGS) $$(SDL_CFLAGS) $$(CXXFLAGS) -I$$(BUILD_DIR)/generated/$(1) \
		$$(GAME_CPP_FLAGS_$(1)) -DARDUINO=10819 -MMD -MP -c $$< -o $$@

$$(BUILD_DIR)/linux/replay-$(1)/%.o: %.cpp
	@mkdir -p $$(@D)
	$$(CXX) $$(CPPFLAGS) $$(CXXFLAGS) -I$$(BUILD_DIR)/generated/$(1) \
		$$(GAME_CPP_FLAGS_$(1)) -DARDUINO=10819 -MMD -MP -c $$< -o $$@

$$(BUILD_DIR)/linux/$(1)/%.o: %.c
	@mkdir -p $$(@D)
	$$(CC) $$(CPPFLAGS) $$(SDL_CFLAGS) -O2 -Wall -Wextra -I$$(BUILD_DIR)/generated/$(1) -MMD -MP -c $$< -o $$@

$$(filter $$(BUILD_DIR)/linux/$(1)/build/generated/$(1)/%,$$(GAME_SDL_OBJECTS_$(1))): $$(GAME_GENERATED_ENTRY_$(1))
$$(filter $$(BUILD_DIR)/linux/replay-$(1)/build/generated/$(1)/%,$$(GAME_REPLAY_OBJECTS_$(1))): $$(GAME_GENERATED_ENTRY_$(1))
$$(GAME_EXTRA_CPP_SOURCES_$(1):%.cpp=$$(BUILD_DIR)/linux/$(1)/%.o): $$(GAME_GENERATED_ENTRY_$(1))
$$(GAME_EXTRA_CPP_SOURCES_$(1):%.cpp=$$(BUILD_DIR)/linux/replay-$(1)/%.o): $$(GAME_GENERATED_ENTRY_$(1))
$$(BUILD_DIR)/linux/$(1)/src/arduboy2/Arduboy2.o: $$(BUILD_DIR)/generated/font5x7.inc
$$(BUILD_DIR)/linux/replay-$(1)/src/arduboy2/Arduboy2.o: $$(BUILD_DIR)/generated/font5x7.inc

$$(GAME_SDL_TARGET_$(1)): $$(GAME_SDL_OBJECTS_$(1))
	@mkdir -p $$(@D)
	$$(CXX) $$^ $$(LINUX_RUNTIME_LDFLAGS) $$(SDL_LIBS) -o $$@

.PHONY: $(1) test-$(1)
$(1): check-upstream check-sdl $$(GAME_SDL_TARGET_$(1))
	$$(GAME_SDL_TARGET_$(1))

test-$(1): check-upstream check-sdl $$(GAME_SDL_TARGET_$(1)) $$(if $$(GAME_REPLAY_SOURCE_$(1)),$$(BUILD_DIR)/$(1)-replay-test)
	$$(if $$(GAME_REPLAY_SOURCE_$(1)),$$(BUILD_DIR)/$(1)-replay-test)
	@save_dir="$$$$(mktemp -d)"; trap 'rm -rf "$$$$save_dir"' EXIT; \
		$$(GAME_SDL_TARGET_$(1)) --headless --frames $$(GAME_SMOKE_FRAMES_$(1)) --save-dir "$$$$save_dir"

ifneq ($$(strip $$(GAME_REPLAY_SOURCE_$(1))),)
$$(BUILD_DIR)/$(1)-replay-test: $$(GAME_REPLAY_OBJECTS_$(1))
	@mkdir -p $$(@D)
	$$(CXX) $$^ $$(LDFLAGS) -o $$@
endif

PORT_BUILD_TARGETS += $$(GAME_SDL_TARGET_$(1))
PORT_TEST_TARGETS += test-$(1)
PORT_DEPENDS += $$(GAME_SDL_OBJECTS_$(1):.o=.d) \
	$$(if $$(GAME_REPLAY_SOURCE_$(1)),$$(GAME_REPLAY_OBJECTS_$(1):.o=.d))
endef

$(foreach game,$(GAME_IDS),$(eval $(call GAME_template,$(game))))

ifneq ($(strip $(GAME)),)
SELECTED_GAME_TARGETS := $(GAME_SDL_TARGET_$(GAME))
endif

ifneq ($(filter build run,$(MAKECMDGOALS)),)
ifeq ($(strip $(GAME)),)
$(error build 和 run 目标必须通过 GAME=<game-id> 选择游戏)
endif
endif

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
	$(BUILD_DIR)/src/compat/ArduboyTones.o \
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

.PHONY: all build run test clean check-upstream check-sdl

all: check-upstream check-sdl $(if $(strip $(GAME)),$(SELECTED_GAME_TARGETS),$(PORT_BUILD_TARGETS))
	$(if $(strip $(GAME)),$(SELECTED_GAME_TARGETS),@:)

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

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

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

clean:
	rm -rf $(BUILD_DIR)

-include $(BUILD_DIR)/tests/framebuffer_test.d \
	$(if $(PORT_DEPENDS_OVERRIDE),$(PORT_DEPENDS_OVERRIDE),$(PORT_DEPENDS))
