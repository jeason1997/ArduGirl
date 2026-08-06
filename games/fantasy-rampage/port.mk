FANTASY_RAMPAGE_UPSTREAM := third_party/Fantasy-Rampage/FRMP_AB
FANTASY_RAMPAGE_GENERATED_DIR := $(BUILD_DIR)/generated/fantasy-rampage
FANTASY_RAMPAGE_GENERATED := $(FANTASY_RAMPAGE_GENERATED_DIR)/FRMP_AB.ino
FANTASY_RAMPAGE_PATCHES := $(wildcard games/fantasy-rampage/patches/[0-9][0-9][0-9][0-9]-*.patch)
FANTASY_RAMPAGE_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp \
	games/fantasy-rampage/entry.cpp
FANTASY_RAMPAGE_OBJECTS := $(FANTASY_RAMPAGE_SOURCES:%.cpp=$(BUILD_DIR)/fantasy-rampage/%.o)
FANTASY_RAMPAGE_TARGET := $(BUILD_DIR)/fantasy-rampage-sdl

$(FANTASY_RAMPAGE_GENERATED): $(FANTASY_RAMPAGE_UPSTREAM)/FRMP_AB.ino $(FANTASY_RAMPAGE_PATCHES)
	@rm -rf $(FANTASY_RAMPAGE_GENERATED_DIR).tmp
	@mkdir -p $(FANTASY_RAMPAGE_GENERATED_DIR).tmp
	@for source_file in $(FANTASY_RAMPAGE_UPSTREAM)/*.h $(FANTASY_RAMPAGE_UPSTREAM)/*.ino; do \
		sed 's/\r$$//' "$$source_file" > $(FANTASY_RAMPAGE_GENERATED_DIR).tmp/$$(basename "$$source_file"); \
	done
	@for patch_file in $(FANTASY_RAMPAGE_PATCHES); do \
		patch --silent -d $(FANTASY_RAMPAGE_GENERATED_DIR).tmp -p0 < $$patch_file || \
		{ patch_status=$$?; rm -rf $(FANTASY_RAMPAGE_GENERATED_DIR).tmp; exit $$patch_status; }; \
	done
	@rm -rf $(FANTASY_RAMPAGE_GENERATED_DIR)
	@mv $(FANTASY_RAMPAGE_GENERATED_DIR).tmp $(FANTASY_RAMPAGE_GENERATED_DIR)

$(BUILD_DIR)/fantasy-rampage/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -I$(FANTASY_RAMPAGE_UPSTREAM) -MMD -MP -c $< -o $@

$(BUILD_DIR)/fantasy-rampage/games/fantasy-rampage/entry.o: $(FANTASY_RAMPAGE_GENERATED)
$(BUILD_DIR)/fantasy-rampage/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

$(FANTASY_RAMPAGE_TARGET): $(FANTASY_RAMPAGE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LINUX_RUNTIME_LDFLAGS) $(SDL_LIBS) -o $@

.PHONY: fantasy-rampage check-fantasy-rampage-upstream test-fantasy-rampage

check-fantasy-rampage-upstream:
	@test -f $(FANTASY_RAMPAGE_UPSTREAM)/FRMP_AB.ino || \
		(printf '%s\\n' '缺少 Fantasy Rampage 子模块，请运行：git submodule update --init --recursive' && false)
	@test -f third_party/ATMlib/src/ATMlib.cpp || \
		(printf '%s\\n' '缺少 ATMlib 子模块，请运行：git submodule update --init --recursive' && false)

fantasy-rampage: check-fantasy-rampage-upstream check-sdl $(FANTASY_RAMPAGE_TARGET)
	$(FANTASY_RAMPAGE_TARGET)

test-fantasy-rampage: check-fantasy-rampage-upstream check-sdl $(FANTASY_RAMPAGE_TARGET)
	@save_dir="$$(mktemp -d)"; trap 'rm -rf "$$save_dir"' EXIT; \
		$(FANTASY_RAMPAGE_TARGET) --headless --frames 240 --save-dir "$$save_dir"

PORT_BUILD_TARGETS += $(FANTASY_RAMPAGE_TARGET)
PORT_TEST_TARGETS += test-fantasy-rampage
PORT_DEPENDS += $(FANTASY_RAMPAGE_OBJECTS:.o=.d)
