ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= writer.cpp
LOCAL_MODULE := writer
LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := \
	libandroid_runtime \
	libnativehelper \
	libcutils \
	libutils \
#	libcameraservice \
#	libmedia \
#	libdl \
#	libui \
#	liblog \
#    libicuuc \
#    libicui18n \
 #   libsqlite

#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)

endif  #TARGET_SIMULATOR != true
