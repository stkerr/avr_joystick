MCU          = atmega32u4
ARCH         = AVR8
BOARD        = USER 
F_CPU        = 8000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = joystick
SRC          = $(wildcard src/*.c) \
				$(LUFA_SRC_USB) \
				$(LUFA_SRC_USBCLASS)
LUFA_PATH    = lufa-LUFA-140928/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -Isrc/ -Werror
LD_FLAGS     =


# Default target
all: avrdude

run: all
	$(MAKE) -C host_app run

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
