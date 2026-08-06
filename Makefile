PLATFORM ?= linux
PLATFORM_MAKEFILE := platform/$(PLATFORM)/Makefile

.DEFAULT_GOAL := all

.PHONY: all

all:
	$(MAKE) -f $(PLATFORM_MAKEFILE) $@

.DEFAULT:
	$(MAKE) -f $(PLATFORM_MAKEFILE) $@
