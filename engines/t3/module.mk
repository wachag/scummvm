MODULE := engines/t3

MODULE_OBJS := \
	metaengine.o \
	gfx.o \
	gfx_opengl.o \
	gfx_opengl_shaders.o \
	ttarchive.o \
	ttmetafile.o \
	t3.o \
	t3seakablereadstream.o \
	texture.o \
	utils\blowfish_ttarch.o

ifdef USE_TINYGL
MODULE_OBJS += \
	gfx_tinygl.o
endif

MODULE_DIRS += \
	engines/t3

# This module can be built as a plugin
ifeq ($(ENABLE_T3), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
