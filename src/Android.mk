LOCAL_PATH:= $(call my-dir)


##################################################
###                 great media                ###
##################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/Media/*.c*)) \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/Media/Camera/*.c*)) \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/Media/Audio/*.c*)) \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/RtpTrans/*.c*)) \
    	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/RtpTrans/rtplib/jrtplib/*.c*)) \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/Common/*.c*))

	#JCrypto rely libbinder libmedia
LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
	libnativehelper \
	liblog \
	libbinder \
    libutils \
    libcutils \
	libcamera_client \
	libdl


LOCAL_C_INCLUDES += \
    frameworks/base/core/jni \
    frameworks/av/media/libmedia \
    frameworks/av/media/libstagefright \
    frameworks/native/include/media/openmax \
	$(LOCAL_PATH)/Common \
	$(LOCAL_PATH)/Media \
	$(LOCAL_PATH)/Media/Codec \
	$(LOCAL_PATH)/Media/Camera \
	$(LOCAL_PATH)/Media/include \
	$(LOCAL_PATH)/RtpTrans	\
	$(LOCAL_PATH)/RtpTrans/rtplib/jrtplib	\
    $(call include-path-for, libhardware)/hardware \
    $(call include-path-for, corecg graphics)

	LOCAL_C_INCLUDES += external/stlport/stlport bionic

LOCAL_CFLAGS +=


LOCAL_SHARED_LIBRARIES += libstlport


LOCAL_MODULE:= libgreat_media


include $(BUILD_SHARED_LIBRARY)

LOCAL_PRELINK_MODULE:= false

include $(call all-makefiles-under,$(LOCAL_PATH))




