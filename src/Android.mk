LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/Media/*.c*)) \
    $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/rtplib/*.c*))

LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libnativehelper \
    libutils \
    libbinder \
    libmedia \
    libskia \
    libui \
    liblog \
    libcutils \
    libgui \
    libstagefright \
    libstagefright_foundation \
    libcamera_client \
    libmtp \
    libusbhost \
    libexif \
    libstagefright_amrnb_common \

LOCAL_REQUIRED_MODULES := \
    libexif_jni

LOCAL_STATIC_LIBRARIES := \
    libstagefright_amrnbenc

LOCAL_C_INCLUDES += \
    external/jhead \
    external/tremor/Tremor \
    external/stlport/stlport \
    frameworks/base/core/jni \
    frameworks/av/media/libmedia \
    frameworks/av/media/libstagefright \
    frameworks/av/media/libstagefright/codecs/amrnb/enc/src \
    frameworks/av/media/libstagefright/codecs/amrnb/common \
    frameworks/av/media/libstagefright/codecs/amrnb/common/include \
    frameworks/av/media/mtp \
    frameworks/native/include/media/openmax \
    $(call include-path-for, libhardware)/hardware \
    system/media/camera/include \
    $(PV_INCLUDES) \
    $(JNI_H_INCLUDE) \
    $(call include-path-for, corecg graphics)

LOCAL_C_INCLUDES += external/stlport/stlport bionic

LOCAL_CFLAGS +=

LOCAL_LDLIBS := -lpthread
LOCAL_SHARED_LIBRARIES += libstlport


LOCAL_MODULE:= libgreat_media

include $(BUILD_SHARED_LIBRARY)

LOCAL_PRELINK_MODULE:= false

include $(call all-makefiles-under,$(LOCAL_PATH))


