PLATFORM ?= linux
PLATFORM_MAKEFILE := platform/$(PLATFORM)/Makefile

.DEFAULT_GOAL := all

.PHONY: all FORCE

all:
	$(MAKE) -f $(PLATFORM_MAKEFILE) $@

FORCE:

%: FORCE
	$(MAKE) -f $(PLATFORM_MAKEFILE) $@

# 模式转发规则不得被 Make 用来尝试重建当前构建文件。
Makefile: ;
