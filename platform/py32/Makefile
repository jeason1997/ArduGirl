BUILD_DIR ?= build
.DEFAULT_GOAL := all
VENDOR_DIR := platform/py32/vendor
GAME_PREPARER := tools/prepare_game.py

ifeq ($(strip $(GAME)),)
ifneq ($(filter test,$(MAKECMDGOALS)),test)
$(error 必须通过 GAME=<game-id> 选择游戏)
endif
else
ARDUGIRL_GAME_ENTRY := $(shell python $(GAME_PREPARER) $(GAME) --field entry_cpp)
ARDUGIRL_GAME_INCLUDE := $(shell python $(GAME_PREPARER) $(GAME) --field include_dir)
ARDUGIRL_GAME_GENERATED := $(shell python $(GAME_PREPARER) $(GAME) --field generated_entry)
ARDUGIRL_GAME_MANIFEST := $(shell python $(GAME_PREPARER) $(GAME) --field manifest)
endif

ifeq ($(OS),Windows_NT)
TOOLCHAIN_DIR ?= C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin
CC := "$(TOOLCHAIN_DIR)/arm-none-eabi-gcc.exe"
CXX := "$(TOOLCHAIN_DIR)/arm-none-eabi-g++.exe"
OBJCOPY := "$(TOOLCHAIN_DIR)/arm-none-eabi-objcopy.exe"
SIZE := "$(TOOLCHAIN_DIR)/arm-none-eabi-size.exe"
OPENOCD := $(VENDOR_DIR)/tools/openocd/bin/openocd.exe
OPENOCD_SCRIPTS := $(VENDOR_DIR)/tools/openocd/scripts
OPENOCD_INSTALL := powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(VENDOR_DIR)/scripts/install-openocd.ps1"
else
CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size
OPENOCD := $(VENDOR_DIR)/tools/openocd/bin/openocd
OPENOCD_SCRIPTS := $(VENDOR_DIR)/tools/openocd/share/openocd/scripts
OPENOCD_INSTALL := sh "$(VENDOR_DIR)/scripts/install-openocd.sh"
endif

CPU_FLAGS := -mcpu=cortex-m0plus -mthumb
DEFINES := -DPY32F002Ax5 -DARDUINO=10819
INCLUDES := -Iinclude -I$(BUILD_DIR)/generated -I$(ARDUGIRL_GAME_INCLUDE) \
	-Iplatform/py32/include -isystem $(VENDOR_DIR)/Drivers/CMSIS/Include \
	-isystem $(VENDOR_DIR)/Drivers/CMSIS/Device/PY32F0xx/Include \
	-isystem $(VENDOR_DIR)/Drivers/PY32F0xx_HAL_Driver/Inc
CXXFLAGS := $(CPU_FLAGS) -std=c++17 -Os -g3 -ffunction-sections -fdata-sections \
	-fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit \
	-fsigned-char \
	-Wall -Wextra -Wpedantic $(DEFINES) $(INCLUDES) -MMD -MP
CFLAGS := $(CPU_FLAGS) -Os -g3 -ffunction-sections -fdata-sections \
	-Wall -Wextra $(DEFINES) $(INCLUDES) -MMD -MP

OUTPUT_DIR := $(BUILD_DIR)/py32/$(GAME)
OBJECT_DIR := $(OUTPUT_DIR)/obj
TARGET := $(OUTPUT_DIR)/ardugirl-$(GAME)-py32f002a
LDSCRIPT := $(VENDOR_DIR)/Core/Startup/py32f002a_32k_4k.ld
OPENOCD_TARGET := $(VENDOR_DIR)/Core/OpenOCD/py32f002a_32k.cfg

CPP_SOURCES := src/core/framebuffer.cpp src/compat/ATMlib.cpp \
	src/compat/ArduboyPlaytune.cpp src/compat/stdio.cpp \
	src/arduboy2/Arduboy2.cpp src/arduboy2/Sprites.cpp \
	src/compat/EEPROM.cpp platform/py32/src/runtime.cpp platform/py32/src/platform.cpp \
	platform/py32/src/display_st7789.cpp $(ARDUGIRL_GAME_ENTRY)
C_SOURCES := $(VENDOR_DIR)/Core/System/system_py32f0xx.c \
	$(VENDOR_DIR)/Drivers/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_utils.c
CPP_OBJECTS := $(CPP_SOURCES:%.cpp=$(OBJECT_DIR)/%.o)
C_OBJECTS := $(C_SOURCES:%.c=$(OBJECT_DIR)/%.o)
STARTUP_OBJECT := $(OBJECT_DIR)/$(VENDOR_DIR)/Core/Startup/startup_py32f002xx.o
OBJECTS := $(CPP_OBJECTS) $(C_OBJECTS) $(STARTUP_OBJECT)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all test flash openocd clean

all: $(TARGET).elf $(TARGET).hex $(TARGET).bin

test:
	python platform/py32/tools/test_games.py

$(ARDUGIRL_GAME_GENERATED): $(ARDUGIRL_GAME_MANIFEST) $(GAME_PREPARER)
	python $(GAME_PREPARER) $(GAME) --prepare

$(OBJECT_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJECT_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(STARTUP_OBJECT): $(VENDOR_DIR)/Core/Startup/startup_py32f002xx.s
	@mkdir -p $(@D)
	$(CC) $(CPU_FLAGS) -x assembler-with-cpp -MMD -MP -c $< -o $@

$(OBJECT_DIR)/$(ARDUGIRL_GAME_ENTRY:.cpp=.o): $(ARDUGIRL_GAME_GENERATED)
$(OBJECT_DIR)/src/arduboy2/Arduboy2.o: $(BUILD_DIR)/generated/font5x7.inc

$(BUILD_DIR)/generated/font5x7.inc: third_party/Arduboy2/src/Arduboy2Data.cpp
	@mkdir -p $(@D)
	@sed -n '/font5x7\[\] = {/,/^};/p' $< | sed '1d;$$d' > $@

$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(CXX) $(CPU_FLAGS) -nostdlib -T$(LDSCRIPT) -Wl,--gc-sections \
		-Wl,-Map=$(TARGET).map,--cref $(OBJECTS) -lgcc -o $@
	$(SIZE) $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

openocd:
	$(OPENOCD_INSTALL)

flash: all openocd
	$(OPENOCD) -s $(OPENOCD_SCRIPTS) -f interface/cmsis-dap.cfg \
		-f $(OPENOCD_TARGET) -c "program $(TARGET).elf verify reset exit"

clean:
	@rm -rf $(OUTPUT_DIR)

-include $(DEPS)
