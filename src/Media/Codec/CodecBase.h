

#ifndef _ANDROID_CODEC_BASE_H_
#define _ANDROID_CODEC_BASE_H_


#include <gui/Surface.h>


#include "media_MediaCodec.h"

#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)

#define HEIGHT_VERSION false

namespace android 
{

struct CodecBase : public RefBase 
{
    CodecBase( const char *name, bool nameIsType, bool encoder);

    status_t CreateCodec(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags);

	status_t startCodec();
	status_t stopCodec();
	
	status_t addBuffer(char*data, int len);
	status_t getCodecBuffer();

	
protected:
    virtual ~CodecBase();
	

private:
    sp<JMediaCodec> 	mCodec;
	
	int*		mpInputBufferPoint;
	int*		mpOutputBufferPoint;
	int				miInputArrayLen;
	int				miOutputArrayLen;
	bool			mbRunning;
	
	pthread_mutex_t mMutex;
	pthread_cond_t	mCond;    
	JNIEnv 			*mpEnv;
	int 			mCount;

    DISALLOW_EVIL_CONSTRUCTORS(CodecBase);
};

}  // namespace android

#endif  // _ANDROID_MEDIA_MEDIACODEC_H_
