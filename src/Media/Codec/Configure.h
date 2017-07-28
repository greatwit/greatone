#ifndef _CONFIGURE_BASE_H_
#define _CONFIGURE_BASE_H_


#define HEIGHT_VERSION true

#define ALOGTEST(...)		__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)


#define PROPERTY_KEY_MAX    32
#define PROPERTY_VALUE_MAX  92


#endif


