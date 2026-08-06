HELMETS_HORDES_UPSTREAM := third_party/Helmets-Hordes/HEHO_AB
HELMETS_HORDES_GENERATED_DIR := $(BUILD_DIR)/generated/helmets-hordes
HELMETS_HORDES_GENERATED := $(HELMETS_HORDES_GENERATED_DIR)/HEHO_AB.ino
HELMETS_HORDES_PATCHES := $(wildcard games/helmets-hordes/patches/[0-9][0-9][0-9][0-9]-*.patch)
HELMETS_HORDES_SOURCES := \
	$(SDL_COMMON_SOURCES) \
	src/arduboy2/Arduboy2.cpp \
	src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp \
	games/helmets-hordes/entry.cpp
HELMETS_HORDES_OBJECTS := $(HELMETS_HORDES_SOURCES:%.cpp=$(BUILD_DIR)/helmets-hordes/%.o)
HELMETS_HORDES_TARGET := $(BUILD_DIR)/helmets-hordes-sdl

$(HELMETS_HORDES_GENERATED): $(HELMETS_HORDES_UPSTREAM)/HEHO_AB.ino $(HELMETS_HORDES_PATCHES)
	@rm -rf $(HELMETS_HORDES_GENERATED_DIR).tmp
	@mkdir -p $(HELMETS_HORDES_GENERATED_DIR).tmp
	@for source_file in $(HELMETS_HORDES_UPSTREAM)/*.h $(HELMETS_HORDES_UPSTREAM)/*.ino; do \
		sed 's/\r$$//' "$$source_file" > $(HELMETS_HORDES_GENERATED_DIR).tmp/$$(basename "$$source_file"); \
	done
	@for patch_file in $(HELMETS_HORDES_PATCHES); do \
		patch --silent -d $(HELMETS_HORDES_GENERATED_DIR).tmp -p0 < $$patch_file || \
		{ patch_status=$$?; rm -rf $(HELMETS_HORDES_GENERATED_DIR).tmp; exit $$patch_status; }; \
	done
	@rm -rf $(HELMETS_HORDES_GENERATED_DIR)
	@mv $(HELMETS_HORDES_GENERATED_DIR).tmp $(HELMETS_HORDES_GENERATED_DIR)

$(BUILD_DIR)/helmets-hordes/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -I$(HELMETS_HORDES_UPSTREAM) -MMD -MP -c $< -o $@

$(BUILD_DIR)/helmets-hordes/games/helmets-hordes/entry.o: $(HELMETS_HORDES_GENERATED)
$(BUILD_DIR)/helmets-hordes/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

$(HELMETS_HORDES_TARGET): $(HELMETS_HORDES_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LINUX_RUNTIME_LDFLAGS) $(SDL_LIBS) -o $@

.PHONY: helmets-hordes check-helmets-hordes-upstream test-helmets-hordes

check-helmets-hordes-upstream:
	@test -f $(HELMETS_HORDES_UPSTREAM)/HEHO_AB.ino || \
		(printf '%s\\n' '缺少 Helmets & Hordes 子模块，请运行：git submodule update --init --recursive' && false)
	@test -f third_party/ATMlib/src/ATMlib.cpp || \
		(printf '%s\\n' '缺少 ATMlib 子模块，请运行：git submodule update --init --recursive' && false)

helmets-hordes: check-helmets-hordes-upstream check-sdl $(HELMETS_HORDES_TARGET)
	$(HELMETS_HORDES_TARGET)

test-helmets-hordes: check-helmets-hordes-upstream check-sdl $(HELMETS_HORDES_TARGET)
	@save_dir="$$(mktemp -d)"; trap 'rm -rf "$$save_dir"' EXIT; \
		$(HELMETS_HORDES_TARGET) --headless --frames 240 --save-dir "$$save_dir"

PORT_BUILD_TARGETS += $(HELMETS_HORDES_TARGET)
PORT_TEST_TARGETS += test-helmets-hordes
PORT_DEPENDS += $(HELMETS_HORDES_OBJECTS:.o=.d)
