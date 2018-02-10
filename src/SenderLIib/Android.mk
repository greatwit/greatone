LOCAL_PATH:= $(call my-dir)


##################################################
###                 great media                ###
##################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/*.c*)) \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/../RtpTrans/*.c*)) \
    	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/../RtpTrans/rtplib/jrtplib/*.c*)) \
	$(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/../Common/*.c*))

	#JCrypto rely libbinder libmedia
	#libgui:need for android6 camera graphic buffer
LOCAL_SHARED_LIBRARIES := \
						libandroid_runtime \
						libnativehelper \
						liblog


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../Common \
	$(LOCAL_PATH)/../RtpTrans	\
	$(LOCAL_PATH)/../RtpTrans/rtplib/jrtplib

#LOCAL_CFLAGS +=

LOCAL_LDFLAGS := -Wl,--hash-style=sysv

#need for android4 compile, no for android6
#LOCAL_SHARED_LIBRARIES += libstlport 


LOCAL_MODULE:= libgreat_sender


include $(BUILD_SHARED_LIBRARY)

LOCAL_PRELINK_MODULE:= false

include $(call all-makefiles-under,$(LOCAL_PATH))




