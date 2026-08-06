PLATFORM ?= linux
PLATFORM_MAKEFILE := platform/$(PLATFORM)/Makefile

.DEFAULT_GOAL := all

.PHONY: all test test-terminal smoke clean py32 flash-py32

all test test-terminal smoke clean:
	$(MAKE) -f $(PLATFORM_MAKEFILE) $@

py32:
	$(MAKE) -f platform/py32/Makefile GAME=$(PY32_GAME)

flash-py32:
	$(MAKE) -f platform/py32/Makefile GAME=$(PY32_GAME) flash

%:
	$(MAKE) -f $(PLATFORM_MAKEFILE) $@
