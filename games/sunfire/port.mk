SUNFIRE_UPSTREAM := third_party/Sunfire/SUNF_AB
SUNFIRE_GENERATED_DIR := $(BUILD_DIR)/generated/sunfire
SUNFIRE_GENERATED := $(SUNFIRE_GENERATED_DIR)/SUNF_AB.ino
SUNFIRE_PATCHES := $(wildcard games/sunfire/patches/[0-9][0-9][0-9][0-9]-*.patch)
SUNFIRE_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp \
	games/sunfire/entry.cpp
SUNFIRE_OBJECTS := $(SUNFIRE_SOURCES:%.cpp=$(BUILD_DIR)/sunfire/%.o)
SUNFIRE_TARGET := $(BUILD_DIR)/sunfire-sdl

$(SUNFIRE_GENERATED): $(SUNFIRE_UPSTREAM)/SUNF_AB.ino $(SUNFIRE_PATCHES)
	@rm -rf $(SUNFIRE_GENERATED_DIR).tmp
	@mkdir -p $(SUNFIRE_GENERATED_DIR).tmp
	@for source_file in $(SUNFIRE_UPSTREAM)/*.h $(SUNFIRE_UPSTREAM)/*.ino; do \
		sed 's/\r$$//' "$$source_file" > $(SUNFIRE_GENERATED_DIR).tmp/$$(basename "$$source_file"); \
	done
	@for patch_file in $(SUNFIRE_PATCHES); do \
		patch --silent -d $(SUNFIRE_GENERATED_DIR).tmp -p0 < $$patch_file || \
		{ patch_status=$$?; rm -rf $(SUNFIRE_GENERATED_DIR).tmp; exit $$patch_status; }; \
	done
	@rm -rf $(SUNFIRE_GENERATED_DIR)
	@mv $(SUNFIRE_GENERATED_DIR).tmp $(SUNFIRE_GENERATED_DIR)

$(BUILD_DIR)/sunfire/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -I$(SUNFIRE_UPSTREAM) -MMD -MP -c $< -o $@

$(BUILD_DIR)/sunfire/games/sunfire/entry.o: $(SUNFIRE_GENERATED)
$(BUILD_DIR)/sunfire/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

$(SUNFIRE_TARGET): $(SUNFIRE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LINUX_RUNTIME_LDFLAGS) $(SDL_LIBS) -o $@

.PHONY: sunfire check-sunfire-upstream test-sunfire

check-sunfire-upstream:
	@test -f $(SUNFIRE_UPSTREAM)/SUNF_AB.ino || \
		(printf '%s\\n' '缺少 Sunfire 子模块，请运行：git submodule update --init --recursive' && false)
	@test -f third_party/ATMlib/src/ATMlib.cpp || \
		(printf '%s\\n' '缺少 ATMlib 子模块，请运行：git submodule update --init --recursive' && false)

sunfire: check-sunfire-upstream check-sdl $(SUNFIRE_TARGET)
	$(SUNFIRE_TARGET)

test-sunfire: check-sunfire-upstream check-sdl $(SUNFIRE_TARGET)
	@save_dir="$$(mktemp -d)"; trap 'rm -rf "$$save_dir"' EXIT; \
		$(SUNFIRE_TARGET) --headless --frames 240 --save-dir "$$save_dir"

PORT_BUILD_TARGETS += $(SUNFIRE_TARGET)
PORT_TEST_TARGETS += test-sunfire
PORT_DEPENDS += $(SUNFIRE_OBJECTS:.o=.d)
