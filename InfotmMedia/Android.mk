#
#
#
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/config.mk
ifeq ($(IM_SUPPORT_audiodec), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaAudioDec.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_bitblt), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaBlt.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_convert), true)
endif

ifeq ($(IM_SUPPORT_buffalloc), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaBuffalloc.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_camera), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaCamera.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_cameralib_stub), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaCameralib_stub.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_cameralib_usb), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaCameralib_usb.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_cameralib_isp), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaCameralib_isp.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_cameralib_camdrv), true)
endif

ifeq ($(IM_SUPPORT_camdrv), true)
endif

ifeq ($(IM_SUPPORT_cameralib_camif), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaCameralib_camif.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_devmmu), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaDevMMU.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_ffmpeg-0.8), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS :=  \
   packages/libswscale.so \
   packages/libavformat.so \
   packages/libavcodec.so \
   packages/libavutil.so 
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_ffmpeg-0.11), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS :=  \
   packages/libim_swscale.so \
   packages/libim_avformat.so \
   packages/libim_avcodec.so \
   packages/libim_avutil.so \
   packages/libim_swresample.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_foundations), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS :=  \
	packages/InfotmMediaFoundations.a \
	packages/InfotmMediaFoundations_c.a
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_G2D), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaG2D.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_g3d_mali400), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS :=  \
			packages/libUMP.so \
 			packages/libMali.so 
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
PRODUCT_COPY_FILES +=  \
	$(LOCAL_PATH)/packages/libEGL_mali.so:system/lib/egl/libEGL_mali.so \
	$(LOCAL_PATH)/packages/libGLESv1_CM_mali.so:system/lib/egl/libGLESv1_CM_mali.so \
	$(LOCAL_PATH)/packages/libGLESv2_mali.so:system/lib/egl/libGLESv2_mali.so
endif

ifeq ($(IM_SUPPORT_IDS), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaIDS.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_IPC), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaIPC.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_jpegdec), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaJpegDec.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_jpegenc), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaJpegenc.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_parser), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaParser.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_parserlib), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaParserlib.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_refpointer), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaRefptr.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_utlzstatc), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaUtlzstatc.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_vdec_lib_g1), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := \
	packages/libg1dec_avs.so \
	packages/libg1dec_h264.so \
	packages/libg1dec_mpeg2.so \
	packages/libg1dec_pp.so \
	packages/libg1dec_vc1.so \
	packages/libg1dec_vp8.so \
	packages/libg1dec_vp6.so \
	packages/libg1dec_dwl.so \
	packages/libg1dec_mpeg4.so \
	packages/libg1dec_jpeg.so \
	packages/libg1dec_rv.so 
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_venc_lib_8270), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/lib8270enc.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_videodec), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaVideoDec.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_videodec2), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaVideoDec2.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_wmapro), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libwmapro.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_videoenc), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaVideoEnc.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_image), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaImage.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif

ifeq ($(IM_SUPPORT_dbt), true)
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := packages/libInfotmMediaDBT.so
LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif







