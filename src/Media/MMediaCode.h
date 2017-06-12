
#ifndef _ANDROID_MMEDIA_CODEC_H_
#define _ANDROID_MMEDIA_CODEC_H_

#include "jni.h"

#include <media/hardware/CryptoAPI.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

#define HEIGHT_VERSION false

namespace android 
{

struct ALooper;
struct AMessage;
struct AString;
struct ICrypto;
struct MediaCodec;

#if HEIGHT_VERSION
struct IGraphicBufferProducer;
class Surface;
#else
struct ISurfaceTexture;
struct SurfaceTextureClient;
#endif


#define ALOGTEST(...)  __android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)

// Keep these in sync with their equivalents in MediaCodec.java !!!
enum 
{
    DEQUEUE_INFO_TRY_AGAIN_LATER            = -1,
    DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED      = -2,
    DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED     = -3,
};

struct MMediaCodec : public RefBase 
{
    MMediaCodec( JNIEnv *env, jobject thiz, const char *name, bool nameIsType, bool encoder);

    status_t initCheck() const;

    status_t configure(
            const sp<AMessage> &format,
	#if HEIGHT_VERSION
	    const sp<IGraphicBufferProducer> &bufferProducer,
	#else
	    const sp<ISurfaceTexture> &surfaceTexture,
	#endif
            const sp<ICrypto> &crypto,
            int flags);

    status_t start();
    status_t stop();

    status_t flush();

    status_t queueInputBuffer(
            size_t index,
            size_t offset, size_t size, int64_t timeUs, uint32_t flags,
            AString *errorDetailMsg);

    status_t dequeueInputBuffer(size_t *index, int64_t timeoutUs);

    status_t dequeueOutputBuffer( JNIEnv *env, jobject bufferInfo, size_t *index, int64_t timeoutUs);

#if HEIGHT_VERSION
    status_t releaseOutputBuffer(size_t index, bool render, bool updatePTS, int64_t timestampNs);
#else
    status_t releaseOutputBuffer(size_t index, bool render);
#endif
    
    status_t getOutputFormat(JNIEnv *env, jobject *format) const;

    status_t getBuffers(
            JNIEnv *env, bool input, jobjectArray *bufArray) const;

    void setVideoScalingMode(int mode);

protected:
    virtual ~JMediaCodec();

private:
    jclass mClass;
    jweak mObject;
    
#if HEIGHT_VERSION
    sp<Surface> mSurfaceTextureClient;
#else
    sp<SurfaceTextureClient> mSurfaceTextureClient;
#endif

    sp<ALooper> mLooper;
    sp<MediaCodec> mCodec;

    DISALLOW_EVIL_CONSTRUCTORS(JMediaCodec);
};

}  // namespace android

#endif  // _ANDROID_MEDIA_MEDIACODEC_H_


