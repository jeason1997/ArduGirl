ARDYNIA_TARGET := $(BUILD_DIR)/ardynia-sdl
ARDYNIA_UPSTREAM_SOURCES := $(filter-out third_party/Ardynia/src/drawBitmap.cpp,$(wildcard third_party/Ardynia/src/*.cpp)) \
	$(filter-out third_party/Ardynia/src/entities/snake.cpp,$(wildcard third_party/Ardynia/src/entities/*.cpp))
ARDYNIA_PATCHED_DRAW_BITMAP := $(BUILD_DIR)/generated/ardynia/src/drawBitmap.cpp
ARDYNIA_PATCHED_SNAKE := $(BUILD_DIR)/generated/ardynia/src/entities/snake.cpp
ARDYNIA_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp \
	games/ardynia/entry.cpp \
	$(ARDYNIA_PATCHED_DRAW_BITMAP) \
	$(ARDYNIA_PATCHED_SNAKE) \
	$(ARDYNIA_UPSTREAM_SOURCES)
ARDYNIA_OBJECTS := $(ARDYNIA_SOURCES:%.cpp=$(BUILD_DIR)/ardynia/%.o)
ARDYNIA_REPLAY_TARGET := $(BUILD_DIR)/ardynia-replay-test
ARDYNIA_REPLAY_SOURCES := \
	tests/ardynia_replay_test.cpp \
	src/core/framebuffer.cpp \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/ArduboyPlaytune.cpp \
	src/compat/EEPROM.cpp \
	$(ARDYNIA_PATCHED_DRAW_BITMAP) \
	$(ARDYNIA_PATCHED_SNAKE) \
	$(ARDYNIA_UPSTREAM_SOURCES)
ARDYNIA_REPLAY_OBJECTS := $(ARDYNIA_REPLAY_SOURCES:%.cpp=$(BUILD_DIR)/ardynia-replay/%.o)
ARDYNIA_CPPFLAGS := $(CPPFLAGS) -iquote third_party/Ardynia/src \
	-iquote third_party/Ardynia/src/entities
ARDYNIA_PATCHED_HEADER := $(BUILD_DIR)/generated/ardynia/src/entityTemplates.h

.PHONY: ardynia check-ardynia-upstream test-ardynia

check-ardynia-upstream:
	@test -f third_party/Ardynia/ardynia.ino || \
		(printf '%s\n' '缺少 Ardynia 子模块，请运行：git submodule update --init --recursive' && false)

$(ARDYNIA_TARGET): $(ARDYNIA_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(ARDYNIA_OBJECTS) $(LINUX_RUNTIME_LDFLAGS) $(SDL_LIBS) -o $@

$(ARDYNIA_REPLAY_TARGET): $(ARDYNIA_REPLAY_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(ARDYNIA_REPLAY_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/ardynia/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(ARDYNIA_CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(BUILD_DIR)/ardynia-replay/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(ARDYNIA_CPPFLAGS) $(CXXFLAGS) -DARDUINO=10819 -MMD -MP -c $< -o $@

$(ARDYNIA_PATCHED_HEADER): third_party/Ardynia/src/entityTemplates.h \
		games/ardynia/patches/0001-entity-template-avoid-negative-shift.patch
	@mkdir -p $(@D)
	@sed 's/\r$$//' $< > $@.base
	@patch --silent --output=$@ $@.base < games/ardynia/patches/0001-entity-template-avoid-negative-shift.patch
	@rm -f $@.base

$(ARDYNIA_PATCHED_DRAW_BITMAP): third_party/Ardynia/src/drawBitmap.cpp \
		games/ardynia/patches/0002-draw-bitmap-use-signed-framebuffer-offset.patch
	@mkdir -p $(@D)
	@sed 's/\r$$//' $< > $@.base
	@patch --silent --output=$@ $@.base < games/ardynia/patches/0002-draw-bitmap-use-signed-framebuffer-offset.patch
	@rm -f $@.base

$(ARDYNIA_PATCHED_SNAKE): third_party/Ardynia/src/entities/snake.cpp \
		games/ardynia/patches/0003-snake-check-bounds-before-tile-read.patch
	@mkdir -p $(@D)
	@sed 's/\r$$//' $< > $@.base
	@patch --silent --output=$@ $@.base < games/ardynia/patches/0003-snake-check-bounds-before-tile-read.patch
	@rm -f $@.base

$(ARDYNIA_OBJECTS) $(ARDYNIA_REPLAY_OBJECTS): $(ARDYNIA_PATCHED_HEADER)

ARDYNIA_ENTITY_TEMPLATE_OBJECTS := \
	$(BUILD_DIR)/ardynia/third_party/Ardynia/src/game.o \
	$(BUILD_DIR)/ardynia/third_party/Ardynia/src/loadEntity.o \
	$(BUILD_DIR)/ardynia/third_party/Ardynia/src/entities/player.o

$(ARDYNIA_ENTITY_TEMPLATE_OBJECTS): $(BUILD_DIR)/ardynia/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(ARDYNIA_CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -DARDUINO=10819 \
		-iquote third_party/Ardynia/src -include $(ARDYNIA_PATCHED_HEADER) \
		-MMD -MP -c $< -o $@

ARDYNIA_REPLAY_ENTITY_TEMPLATE_OBJECTS := \
	$(BUILD_DIR)/ardynia-replay/third_party/Ardynia/src/game.o \
	$(BUILD_DIR)/ardynia-replay/third_party/Ardynia/src/loadEntity.o \
	$(BUILD_DIR)/ardynia-replay/third_party/Ardynia/src/entities/player.o

$(ARDYNIA_REPLAY_ENTITY_TEMPLATE_OBJECTS): $(BUILD_DIR)/ardynia-replay/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(ARDYNIA_CPPFLAGS) $(CXXFLAGS) -DARDUINO=10819 \
		-iquote third_party/Ardynia/src -include $(ARDYNIA_PATCHED_HEADER) \
		-MMD -MP -c $< -o $@

$(BUILD_DIR)/ardynia/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/ardynia-replay/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

ardynia: check-ardynia-upstream check-sdl $(ARDYNIA_TARGET)
	$(ARDYNIA_TARGET)

test-ardynia: check-ardynia-upstream check-sdl $(ARDYNIA_TARGET) $(ARDYNIA_REPLAY_TARGET)
	$(ARDYNIA_REPLAY_TARGET)
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	$(ARDYNIA_TARGET) --headless --frames 180 --save-dir "$$save_dir"

PORT_TEST_TARGETS += test-ardynia
PORT_BUILD_TARGETS += $(ARDYNIA_TARGET)
PORT_DEPENDS += $(ARDYNIA_OBJECTS:.o=.d) $(ARDYNIA_REPLAY_OBJECTS:.o=.d)
