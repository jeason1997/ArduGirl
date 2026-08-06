ARDUVENTURE_UPSTREAM := third_party/Arduventure/ARDU_AB
ARDUVENTURE_GENERATED := $(BUILD_DIR)/generated/arduventure/ARDU_AB.ino
ARDUVENTURE_GENERATED_DIR := $(BUILD_DIR)/generated/arduventure
ARDUVENTURE_PATCHES := $(wildcard games/arduventure/patches/[0-9][0-9][0-9][0-9]-*.patch)
ARDUVENTURE_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp \
	games/arduventure/entry.cpp
ARDUVENTURE_OBJECTS := $(ARDUVENTURE_SOURCES:%.cpp=$(BUILD_DIR)/arduventure/%.o)
ARDUVENTURE_TARGET := $(BUILD_DIR)/arduventure-sdl
ARDUVENTURE_REPLAY_TARGET := $(BUILD_DIR)/arduventure-replay-test
ARDUVENTURE_REPLAY_SOURCES := \
	tests/arduventure_replay_test.cpp \
	src/core/framebuffer.cpp \
	src/compat/ATMlib.cpp \
	src/compat/ArduboyPlaytune.cpp \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp
ARDUVENTURE_REPLAY_OBJECTS := $(ARDUVENTURE_REPLAY_SOURCES:%.cpp=$(BUILD_DIR)/arduventure-replay/%.o)

$(ARDUVENTURE_GENERATED): $(ARDUVENTURE_UPSTREAM)/ARDU_AB.ino $(ARDUVENTURE_PATCHES)
	@rm -rf $(ARDUVENTURE_GENERATED_DIR).tmp
	@mkdir -p $(ARDUVENTURE_GENERATED_DIR).tmp
	@for source_file in $(ARDUVENTURE_UPSTREAM)/*.h $(ARDUVENTURE_UPSTREAM)/*.ino; do \
		sed 's/\r$$//' "$$source_file" > $(ARDUVENTURE_GENERATED_DIR).tmp/$$(basename "$$source_file"); \
	done
	@for patch_file in $(ARDUVENTURE_PATCHES); do \
		patch --silent -d $(ARDUVENTURE_GENERATED_DIR).tmp -p0 < $$patch_file || \
		{ patch_status=$$?; rm -rf $(ARDUVENTURE_GENERATED_DIR).tmp; exit $$patch_status; }; \
	done
	@rm -rf $(ARDUVENTURE_GENERATED_DIR)
	@mv $(ARDUVENTURE_GENERATED_DIR).tmp $(ARDUVENTURE_GENERATED_DIR)

$(BUILD_DIR)/arduventure/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -I$(ARDUVENTURE_UPSTREAM) -MMD -MP -c $< -o $@

$(BUILD_DIR)/arduventure-replay/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(ARDUVENTURE_UPSTREAM) -MMD -MP -c $< -o $@

$(BUILD_DIR)/arduventure/games/arduventure/entry.o: $(ARDUVENTURE_GENERATED)
$(BUILD_DIR)/arduventure/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/arduventure-replay/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc
$(BUILD_DIR)/arduventure-replay/tests/arduventure_replay_test.o: $(ARDUVENTURE_GENERATED)

$(ARDUVENTURE_TARGET): $(ARDUVENTURE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LINUX_RUNTIME_LDFLAGS) $(SDL_LIBS) -o $@

$(ARDUVENTURE_REPLAY_TARGET): $(ARDUVENTURE_REPLAY_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: arduventure check-arduventure-upstream test-arduventure

check-arduventure-upstream:
	@test -f $(ARDUVENTURE_UPSTREAM)/ARDU_AB.ino || \
		(printf '%s\n' '缺少 Arduventure 子模块，请运行：git submodule update --init --recursive' && false)
	@test -f third_party/ATMlib/src/ATMlib.cpp || \
		(printf '%s\n' '缺少 ATMlib 子模块，请运行：git submodule update --init --recursive' && false)

arduventure: check-arduventure-upstream check-sdl $(ARDUVENTURE_TARGET)
	$(ARDUVENTURE_TARGET)

test-arduventure: check-arduventure-upstream check-sdl $(ARDUVENTURE_TARGET) $(ARDUVENTURE_REPLAY_TARGET)
	$(ARDUVENTURE_REPLAY_TARGET)
	@save_dir="$$(mktemp -d)"; trap 'rm -rf "$$save_dir"' EXIT; \
		$(ARDUVENTURE_TARGET) --headless --frames 180 --save-dir "$$save_dir"

PORT_BUILD_TARGETS += $(ARDUVENTURE_TARGET)
PORT_TEST_TARGETS += test-arduventure
PORT_DEPENDS += $(ARDUVENTURE_OBJECTS:.o=.d) $(ARDUVENTURE_REPLAY_OBJECTS:.o=.d)
