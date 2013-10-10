LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_CFLAGS += -DTARGET_SYSTEM=FS_ANDROID -DTS_VER_MAJOR=4 -DTS_VER_MINOR=1 -DTS_VER_PATCH=0

LOCAL_C_INCLUDES := frameworks/base/include	\
	device/infotm/$(TARGET_BOARD_PLATFORM)/InfotmMedia/include \

LOCAL_CFLAGS := -DTARGET_SYSTEM=FS_ANDROID

LOCAL_SRC_FILES := \
	HDMIHelper.cpp

LOCAL_SHARED_LIBRARIES :=	\
	liblog \
	libcutils \
	libutils \
	libInfotmMediaIDS


LOCAL_STATIC_LIBRARIES :=\
	InfotmMediaFoundations_c	\

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libHDMIHelper

include $(BUILD_SHARED_LIBRARY)
