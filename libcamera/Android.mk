LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS += -D_ANDROID_ -DCAMERA_EXPORTS -DIM_DEBUG 
LOCAL_CFLAGS += -DTARGET_SYSTEM=FS_ANDROID -DTS_VER_MAJOR=4 -DTS_VER_MINOR=1 -DTS_VER_PATCH=0

LOCAL_SRC_FILES:=               \
    CameraHalModule.cpp \
    CameraHal.cpp    \
	Encoder_libjpeg.cpp \

LOCAL_C_INCLUDES += \
    hardware/libhardware/include/hardware \
    frameworks/base/include/ui \
    frameworks/base/include/camera \
    frameworks/base/include/utils \
	frameworks/base/include/ \
	frameworks/base/include/binder \
	frameworks/av/camera/libcameraservice \
	frameworks/av/include/media/stagefright \
	frameworks/av/media/libstagefright/infotm/include	\
	device/infotm/$(TARGET_BOARD_PLATFORM)/InfotmMedia/include	\
	device/infotm/$(TARGET_BOARD_PLATFORM)/InfotmMedia/g3d_mali400/src/ump/include\
	hardware/libhardware/modules/gralloc/\
    external/jpeg \
    external/jhead

LOCAL_SHARED_LIBRARIES := \
    libjpeg \
    libexif	\
	libui \
	libutils \
	libcutils \
	libc	\
	libbinder \
	libmedia  \
	libcamera_client \
	libstagefright	\
	libInfotmMediaCamera	\
	libInfotmMediaJpegenc	\
	libInfotmMediaBuffalloc	\
	libInfotmMediaBlt	\
	libInfotmMediaImage\
#	libInfotmMediaConvert	\

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE:= camera.default
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)
