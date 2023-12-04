BUILD_STATIC_LIBRARY = $(AIMAKE_HOME)/$(TARGET_PLATFORM)/build_static_library.mk
BUILD_SHARED_LIBRARY = $(AIMAKE_HOME)/$(TARGET_PLATFORM)/build_shared_library.mk
BUILD_EXECUTABLE     = $(AIMAKE_HOME)/$(TARGET_PLATFORM)/build_executable.mk
BUILD_ALL            = $(AIMAKE_HOME)/$(TARGET_PLATFORM)/build_all.mk

include $(AIMAKE_HOME)/$(TARGET_PLATFORM)/init.mk
include $(AIMAKEFILE)
