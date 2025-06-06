MODULE := engines/dev7

MODULE_OBJS = \
	dev7.o \
	console.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_DEV7), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
