

#ifndef VIDEO_HARD_CODEC_H_
#define VIDEO_HARD_CODEC_H_


#include "jni.h"

#include <media/hardware/CryptoAPI.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

//using namespace android;
#define HEIGHT_VERSION false


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)


namespace android {

struct ALooper;
struct AMessage;
struct AString;
struct ICrypto;
struct ISurfaceTexture;
struct MediaCodec;
struct SurfaceTextureClient;

class VideoHardCodec
{
    public:
		VideoHardCodec(JNIEnv *env, jobject thiz);
		~VideoHardCodec();

	    void Create(jstring name,         
				jobjectArray keys, jobjectArray values,
				jobject jsurface,
				jobject jcrypto,
				jint flags);

	    void Finished();
	    void Start(jobject bufferInfo);
	    void Stop();
	    
	    void AddBuffer(char*data, int dataLen);

	protected:
		status_t configure(
				const sp<AMessage> &format,
				const sp<ISurfaceTexture> &surfaceTexture,
				const sp<ICrypto> &crypto,
				int flags);
			
		status_t getBuffers(
				JNIEnv *env, bool input, jobjectArray *bufArray) const;
				
		status_t dequeueOutputBuffer( JNIEnv *env, jobject bufferInfo, size_t *index, int64_t timeoutUs);
		status_t dequeueOutputBuffer( JNIEnv *env, size_t *index, int64_t timeoutUs);
		status_t releaseOutputBuffer(size_t index, bool render);
			
	private:
		sp<MediaCodec> mCodec;
		sp<ALooper> mLooper;
		sp<SurfaceTextureClient> mSurfaceTextureClient;
		jclass mClass;
        jweak mObject;
	
	
		JNIEnv *mEnv;
   		jobject mThiz;
		uint8_t*mBufferPoint[2];
		jobject mBufferInfo;
};
}
#endif
