MICROTD_TARGET := $(BUILD_DIR)/microtd-sdl
TERMINAL_MICROTD_TARGET := $(BUILD_DIR)/microtd-terminal
MICROTD_REPLAY_TEST_TARGET := $(BUILD_DIR)/microtd-replay-test
MICROTD_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	games/microtd/entry.cpp
TERMINAL_MICROTD_SOURCES := $(filter-out platform/linux/render.cpp platform/linux/storage.cpp,\
	$(MICROTD_SOURCES:platform/linux/sdl.cpp=platform/linux/terminal.cpp))
TERMINAL_MICROTD_SOURCES += $(LINUX_STORAGE_SOURCE)
MICROTD_OBJECTS := $(MICROTD_SOURCES:%.cpp=$(BUILD_DIR)/microtd/%.o)
TERMINAL_MICROTD_OBJECTS := $(TERMINAL_MICROTD_SOURCES:%.cpp=$(BUILD_DIR)/terminal-microtd/%.o)
MICROTD_REPLAY_TEST_SOURCES := \
	tests/microtd_replay_test.cpp \
	src/core/framebuffer.cpp \
	src/arduboy2/Arduboy2.cpp \
	src/compat/ArduboyPlaytune.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp
MICROTD_REPLAY_TEST_OBJECTS := $(MICROTD_REPLAY_TEST_SOURCES:%.cpp=$(BUILD_DIR)/replay/%.o)

.PHONY: microtd microtd-terminal check-microtd-upstream test-microtd test-microtd-terminal

check-microtd-upstream:
	@test -f third_party/MicroTD/microtd.ino || \
		(printf '%s\n' '缺少 MicroTD 子模块，请运行：git submodule update --init --recursive' && false)

$(MICROTD_TARGET): $(MICROTD_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(MICROTD_OBJECTS) $(LINUX_RUNTIME_LDFLAGS) $(SDL_LIBS) -o $@

$(TERMINAL_MICROTD_TARGET): $(TERMINAL_MICROTD_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(TERMINAL_MICROTD_OBJECTS) $(LINUX_RUNTIME_LDFLAGS) -o $@

$(MICROTD_REPLAY_TEST_TARGET): $(MICROTD_REPLAY_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(MICROTD_REPLAY_TEST_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/microtd/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/terminal-microtd/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/replay/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/generated/microtd/microtd.ino: third_party/MicroTD/microtd.ino \
		games/microtd/game.toml \
		games/microtd/patches/0001-build-selected-tower-return-true.patch \
		tools/prepare_game.py
	python tools/prepare_game.py microtd --prepare

$(BUILD_DIR)/microtd/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/terminal-microtd/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/replay/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/microtd/games/microtd/entry.o: $(BUILD_DIR)/generated/microtd/microtd.ino
$(BUILD_DIR)/terminal-microtd/games/microtd/entry.o: $(BUILD_DIR)/generated/microtd/microtd.ino
$(BUILD_DIR)/replay/tests/microtd_replay_test.o: $(BUILD_DIR)/generated/microtd/microtd.ino

microtd: check-microtd-upstream check-sdl $(MICROTD_TARGET)
	$(MICROTD_TARGET)

microtd-terminal: check-microtd-upstream $(TERMINAL_MICROTD_TARGET)
	$(TERMINAL_MICROTD_TARGET)

test-microtd: check-microtd-upstream check-sdl $(MICROTD_TARGET) $(MICROTD_REPLAY_TEST_TARGET)
	$(MICROTD_REPLAY_TEST_TARGET)
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	$(MICROTD_TARGET) --headless --frames 3 --save-dir "$$save_dir"

test-microtd-terminal: check-microtd-upstream $(TERMINAL_MICROTD_TARGET)
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	output="$$($(TERMINAL_MICROTD_TARGET) --frames 1 --plain --save-dir "$$save_dir")"; \
	printf '%s' "$$output" | grep -q "ArduGirl terminal | MicroTD"; \
	arrow_lines="$$(printf '\033[A' | $(TERMINAL_MICROTD_TARGET) --frames 3 --plain --save-dir "$$save_dir" | wc -l)"; \
	test "$$arrow_lines" -ge 70

PORT_TEST_TARGETS += test-microtd
PORT_TERMINAL_TEST_TARGETS += test-microtd-terminal
PORT_BUILD_TARGETS += $(MICROTD_TARGET)
PORT_DEPENDS += $(MICROTD_OBJECTS:.o=.d) $(TERMINAL_MICROTD_OBJECTS:.o=.d) $(MICROTD_REPLAY_TEST_OBJECTS:.o=.d)
