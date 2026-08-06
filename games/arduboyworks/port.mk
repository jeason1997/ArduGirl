ARDUBOYWORKS_MANIFESTS := $(wildcard games/arduboyworks/*/game.toml)
ifneq ($(strip $(GAME)),)
ARDUBOYWORKS_MANIFESTS := $(filter %/$(GAME)/game.toml,$(ARDUBOYWORKS_MANIFESTS))
endif
ARDUBOYWORKS_GAMES := $(notdir $(patsubst %/,%,$(dir $(ARDUBOYWORKS_MANIFESTS))))
ARDUBOYWORKS_TARGETS := $(ARDUBOYWORKS_GAMES:%=$(BUILD_DIR)/arduboyworks-%-sdl)
ARDUBOYWORKS_TITLE_GAMES := $(notdir $(patsubst %/,%,$(dir $(shell grep -l 'static void drawText(const char \*p, int lines);' third_party/ArduboyWorks/*/title.cpp 2>/dev/null))))
ARDUBOYWORKS_STOP_TONE_GAMES := $(notdir $(patsubst %/,%,$(dir $(shell grep -l stopTone third_party/ArduboyWorks/*/MyArduboy2.h 2>/dev/null))))
ARDUBOYWORKS_PLAY_WAVE_GAMES := $(notdir $(patsubst %/,%,$(dir $(shell grep -l playWave third_party/ArduboyWorks/*/MyArduboy2.h 2>/dev/null))))

define ARDUBOYWORKS_GAME_template
ARDUBOYWORKS_$(1)_TITLE_SOURCE := $$(if $$(filter $(1),$$(ARDUBOYWORKS_TITLE_GAMES)),third_party/ArduboyWorks/$(1)/title.cpp)
ARDUBOYWORKS_$(1)_COMMON_PATCHES := $$(wildcard games/arduboyworks/$(1)/patches/[0-9][0-9][0-9][0-9]-common-*.patch)
ARDUBOYWORKS_$(1)_GAME_PATCHES := $$(wildcard games/arduboyworks/$(1)/patches/[0-9][0-9][0-9][0-9]-game-*.patch)
ARDUBOYWORKS_$(1)_ADAPTER_SOURCE := $$(if $$(wildcard third_party/ArduboyWorks/$(1)/MyArduboy2.h),games/arduboyworks/upstream_adapter.cpp)
ARDUBOYWORKS_$(1)_AUDIO_FLAGS := \
	$$(if $$(filter $(1),$$(ARDUBOYWORKS_STOP_TONE_GAMES)),-DARDUBOYWORKS_HAS_STOP_TONE) \
	$$(if $$(filter $(1),$$(ARDUBOYWORKS_PLAY_WAVE_GAMES)),-DARDUBOYWORKS_HAS_PLAY_WAVE)
ARDUBOYWORKS_$(1)_SOURCES := \
	$$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/compat/EEPROM.cpp \
	src/arduboy2/Sprites.cpp \
	$$(filter-out third_party/ArduboyWorks/$(1)/MyArduboyPlaytune.cpp $$(ARDUBOYWORKS_$(1)_TITLE_SOURCE) $$(if $$(ARDUBOYWORKS_$(1)_COMMON_PATCHES),third_party/ArduboyWorks/$(1)/common.cpp) $$(if $$(ARDUBOYWORKS_$(1)_GAME_PATCHES),third_party/ArduboyWorks/$(1)/game.cpp),$$(wildcard third_party/ArduboyWorks/$(1)/*.cpp)) \
	$$(if $$(ARDUBOYWORKS_$(1)_TITLE_SOURCE),$$(BUILD_DIR)/generated/arduboyworks/$(1)/title.cpp) \
	$$(if $$(ARDUBOYWORKS_$(1)_COMMON_PATCHES),$$(BUILD_DIR)/generated/arduboyworks/$(1)/common.cpp) \
	$$(if $$(ARDUBOYWORKS_$(1)_GAME_PATCHES),$$(BUILD_DIR)/generated/arduboyworks/$(1)/game.cpp) \
	$$(ARDUBOYWORKS_$(1)_ADAPTER_SOURCE) \
	games/arduboyworks/entry.cpp
ARDUBOYWORKS_$(1)_OBJECTS := $$(ARDUBOYWORKS_$(1)_SOURCES:%.cpp=$$(BUILD_DIR)/arduboyworks/$(1)/%.o)

$$(BUILD_DIR)/arduboyworks/$(1)/%.o: %.cpp
	@mkdir -p $$(@D)
	$$(CXX) $$(CPPFLAGS) $$(SDL_CFLAGS) $$(CXXFLAGS) -fpermissive -Wno-narrowing -DARDUINO=10819 -DUSE_ARDUBOY2_LIB \
		$$(ARDUBOYWORKS_$(1)_AUDIO_FLAGS) \
		-Ithird_party/ArduboyWorks -Ithird_party/ArduboyWorks/$(1) -DARDUBOYWORKS_GAME_ID=$(1) \
		-I$$(BUILD_DIR)/generated -DARDUBOYWORKS_ENTRY=\"arduboyworks/$(1).ino\" -MMD -MP -c $$< -o $$@

$$(BUILD_DIR)/arduboyworks/$(1)/src/arduboy2/Arduboy2.o: $$(BUILD_DIR)/generated/font5x7.inc

$$(BUILD_DIR)/generated/arduboyworks/$(1).ino: third_party/ArduboyWorks/$(1)/$(1).ino
	@mkdir -p $$(@D)
	@sed \
		-e 's@^#define callInitFunc.*@#define callInitFunc(idx) (moduleTable[idx].initFunc)()@' \
		-e 's@^#define callUpdateFunc.*@#define callUpdateFunc(idx) (moduleTable[idx].updateFunc)()@' \
		-e 's@^#define callDrawFunc.*@#define callDrawFunc(idx) (moduleTable[idx].drawFunc)()@' \
		$$< > $$@

$$(BUILD_DIR)/generated/arduboyworks/$(1)/title.cpp: third_party/ArduboyWorks/$(1)/title.cpp
	@mkdir -p $$(@D)
	@sed 's/static void drawText(const char \*p, int lines);/static void drawText(const char *p, int16_t y);/' $$< > $$@

$$(BUILD_DIR)/generated/arduboyworks/$(1)/common.cpp: third_party/ArduboyWorks/$(1)/common.cpp $$(ARDUBOYWORKS_$(1)_COMMON_PATCHES)
	@mkdir -p $$(@D)
	@sed 's/\r$$$$//' $$< > $$@.tmp
	@for patch_file in $$(ARDUBOYWORKS_$(1)_COMMON_PATCHES); do patch --silent $$@.tmp < $$$$patch_file || { rm -f $$@.tmp; exit $$$$?; }; done
	@mv $$@.tmp $$@

$$(BUILD_DIR)/generated/arduboyworks/$(1)/game.cpp: third_party/ArduboyWorks/$(1)/game.cpp $$(ARDUBOYWORKS_$(1)_GAME_PATCHES)
	@mkdir -p $$(@D)
	@sed 's/\r$$$$//' $$< > $$@.tmp
	@for patch_file in $$(ARDUBOYWORKS_$(1)_GAME_PATCHES); do patch --silent $$@.tmp < $$$$patch_file || { rm -f $$@.tmp; exit $$$$?; }; done
	@mv $$@.tmp $$@

$$(BUILD_DIR)/arduboyworks/$(1)/games/arduboyworks/entry.o: $$(BUILD_DIR)/generated/arduboyworks/$(1).ino

$$(BUILD_DIR)/arduboyworks-$(1)-sdl: $$(ARDUBOYWORKS_$(1)_OBJECTS)
	@mkdir -p $$(@D)
	$$(CXX) $$^ $$(LINUX_RUNTIME_LDFLAGS) $$(SDL_LIBS) -o $$@

.PHONY: $(1)
$(1): check-arduboyworks-upstream check-sdl $$(BUILD_DIR)/arduboyworks-$(1)-sdl
	$$(BUILD_DIR)/arduboyworks-$(1)-sdl
endef

$(foreach game,$(ARDUBOYWORKS_GAMES),$(eval $(call ARDUBOYWORKS_GAME_template,$(game))))

.PHONY: check-arduboyworks-upstream arduboyworks-build test-arduboyworks

check-arduboyworks-upstream:
	@test -f third_party/ArduboyWorks/README.md || \
		(printf '%s\n' '缺少 ArduboyWorks 子模块，请运行：git submodule update --init --recursive' && false)

arduboyworks-build: check-arduboyworks-upstream check-sdl $(ARDUBOYWORKS_TARGETS)

test-arduboyworks: arduboyworks-build
	@save_dir="$$(mktemp -d)"; \
	trap 'rm -rf "$$save_dir"' EXIT; \
	for game in $(ARDUBOYWORKS_GAMES); do \
		$(BUILD_DIR)/arduboyworks-$$game-sdl --headless --frames 180 --save-dir "$$save_dir" || exit $$?; \
	done

PORT_TEST_TARGETS += test-arduboyworks
PORT_BUILD_TARGETS += $(ARDUBOYWORKS_TARGETS)
PORT_DEPENDS += $(foreach game,$(ARDUBOYWORKS_GAMES),$(ARDUBOYWORKS_$(game)_OBJECTS:.o=.d))

ifeq ($(words $(MAKECMDGOALS)),1)
ARDUBOYWORKS_REQUESTED_GAME := $(filter $(ARDUBOYWORKS_GAMES),$(MAKECMDGOALS) $(GAME))
ifneq ($(ARDUBOYWORKS_REQUESTED_GAME),)
PORT_DEPENDS_OVERRIDE := $(ARDUBOYWORKS_$(ARDUBOYWORKS_REQUESTED_GAME)_OBJECTS:.o=.d)
endif
endif
