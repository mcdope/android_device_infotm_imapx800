#
#
#
IM_SYSTEM := filesystem/android_4_1
IM_ROOT := /storage/sdb/user/leo/android-4.0.3_r1_mid_asic-branch-mid/hardware/infotm/imapx800/InfotmMedia
#IM_RELEASE_ROOT := /storage/sdb/user/leo/android-4.0.3_r1_mid_asic-branch-mid/hardware/infotm/imapx800/InfotmMedia/release
LOCAL_CFLAGS += -DTARGET_SYSTEM=FS_ANDROID -DTS_VER_MAJOR=4 -DTS_VER_MINOR=1 -DTS_VER_PATCH=0
LOCAL_PLATFORM_PATH :=hardware/infotm/imapx800

#
#
#
IM_BUILDER_ROOT := $(IM_ROOT)/builder/$(IM_SYSTEM)
IM_EXTERNAL_ROOT := $(IM_ROOT)/external/$(IM_SYSTEM)
IM_EXTERNAL_WORKCOPY_ROOT := $(IM_ROOT)/external/$(IM_SYSTEM)/workcopy
IM_EXTERNAL_PROJECT_ROOT := $(IM_ROOT)/external/$(IM_SYSTEM)/project

