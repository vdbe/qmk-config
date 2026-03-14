.SILENT:
MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
QMK_FIRMWARE_ROOT ?= $(QMK_USERSPACE)/qmk_firmware

%:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)
