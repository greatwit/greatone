#include "VideoHardCodec.h"

#include <media/stagefright/MediaErrors.h>

#include "android_media_MediaCrypto.h"
#include "android_media_Utils.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"

#if HEIGHT_VERSION
#else
#include <gui/SurfaceTextureClient.h>
#endif

#include <gui/Surface.h>
#include <media/ICrypto.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>

#define TAG "VideoHardCodec"

static void throwCryptoException(JNIEnv *env, status_t err, const char *msg) 
{
    jclass clazz = env->FindClass("android/media/MediaCodec$CryptoException");
    CHECK(clazz != NULL);

    jmethodID constructID =
        env->GetMethodID(clazz, "<init>", "(ILjava/lang/String;)V");
    CHECK(constructID != NULL);

    jstring msgObj = env->NewStringUTF(msg != NULL ? msg : "Unknown Error");

    jthrowable exception =
        (jthrowable)env->NewObject(clazz, constructID, err, msgObj);

    env->Throw(exception);
}

static jint throwExceptionAsNecessary(
        JNIEnv *env, status_t err, const char *msg = NULL) 
{


#if HEIGHT_VERSION
    if (err >= ERROR_DRM_VENDOR_MIN && err <= ERROR_DRM_VENDOR_MAX) {
        // We'll throw our custom MediaCodec.CryptoException
        throwCryptoException(env, err, msg);
        return 0;
    }
#else
    if (err >= ERROR_DRM_WV_VENDOR_MIN && err <= ERROR_DRM_WV_VENDOR_MAX) 
    {
        // We'll throw our custom MediaCodec.CryptoException

        throwCryptoException(env, err, msg);
        return 0;
    }
#endif

    switch (err) 
	{
        case OK:
            return 0;

        case -EAGAIN:
            return DEQUEUE_INFO_TRY_AGAIN_LATER;

        case INFO_FORMAT_CHANGED:
            return DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED;

        case INFO_OUTPUT_BUFFERS_CHANGED:
            return DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED;

        default:
        {
            jniThrowException(env, "java/lang/IllegalStateException", NULL);
            break;
        }
    }

    return 0;
}


VideoHardCodec::VideoHardCodec(JNIEnv *env, jobject thiz)
{
		mEnv  = env;
   		mThiz = thiz;
}

VideoHardCodec::~VideoHardCodec()
{

}

void VideoHardCodec::Create(jstring name,         
	jobjectArray keys, jobjectArray values,
        jobject jsurface,
        jobject jcrypto,
        jint flags)
{
    if (name == NULL) 
    {
        jniThrowException(mEnv, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *tmp = mEnv->GetStringUTFChars(name, NULL);

    if (tmp == NULL) {
        return;
    }

    mCodec = new JMediaCodec(mEnv, mThiz, tmp, true, false);//nameIsType, encoder

    status_t err = mCodec->initCheck();

    mEnv->ReleaseStringUTFChars(name, tmp);
    tmp = NULL;

    if (err != OK) 
    {
        jniThrowException(
                mEnv,
                "java/io/IOException",
                "Failed to allocate component instance");
        return;
    }


    sp<AMessage> format;
    err = ConvertKeyValueArraysToMessage(mEnv, keys, values, &format);

    if (err != OK) 
    {
        jniThrowException(mEnv, "java/lang/IllegalArgumentException", NULL);
        return;
    }

	#if HEIGHT_VERSION
	    sp<IGraphicBufferProducer> bufferProducer;
	    if (jsurface != NULL) 
           {
		sp<Surface> surface(android_view_Surface_getSurface(mEnv, jsurface));
		if (surface != NULL) 
                {
		    bufferProducer = surface->getIGraphicBufferProducer();
		} else {
		    jniThrowException(
		            mEnv,
		            "java/lang/IllegalArgumentException",
		            "The surface has been released");
		    return;
		}
	    }

	    sp<ICrypto> crypto;
	    if (jcrypto != NULL) {
		crypto = JCrypto::GetCrypto(mEnv, jcrypto);
	    }

	    err = mCodec->configure(format, bufferProducer, crypto, flags);
	#else
	    sp<ISurfaceTexture> surfaceTexture;
	    if (jsurface != NULL) 
            {
		sp<Surface> surface(android_view_Surface_getSurface(mEnv, jsurface));
		if (surface != NULL) 
                {
		    surfaceTexture = surface->getSurfaceTexture();
		} 
		else 
		{
		    jniThrowException(
		            mEnv,
		            "java/lang/IllegalArgumentException",
		            "The surface has been released");
		    return;
		}
	    }

	    sp<ICrypto> crypto;
	    if (jcrypto != NULL) {
		crypto = JCrypto::GetCrypto(mEnv, jcrypto);
	    }

	    err = mCodec->configure(format, surfaceTexture, crypto, flags);
	#endif

        throwExceptionAsNecessary(mEnv, err);


}

void VideoHardCodec::Finished()
{
	mCodec->decStrong(mThiz);
}

void VideoHardCodec::Start(jobject bufferInfo)
{
        status_t err = mCodec->start();

	jobjectArray inputBuffers;
	err = mCodec->getBuffers(mEnv, true, &inputBuffers);

	for(int i =0;i<2;i++)
	{
		jobject inputObject 	= mEnv->GetObjectArrayElement(inputBuffers, i);
		mBufferPoint[i] 	= (uint8_t*)mEnv->GetDirectBufferAddress( inputObject);
	}
	
        mBufferInfo = bufferInfo;

        throwExceptionAsNecessary(mEnv, err);
}

void VideoHardCodec::Stop()
{
    status_t err = mCodec->stop();

    throwExceptionAsNecessary(mEnv, err);
}

void VideoHardCodec::AddBuffer(char*data, int dataLen)
{
	size_t inputBufferIndex = 0, outputBufferIndex = 0;
	
	status_t err;
	int mCount = 0;
	AString errorDetailMsg;
	
	
	err = mCodec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("startCodec------------4 err:%d", err);


	uint8_t* inputChar = mBufferPoint[inputBufferIndex];
	memcpy(inputChar, data, dataLen);
	
	err = mCodec->queueInputBuffer(inputBufferIndex, 0, dataLen, mCount * 1000000 / 20, 0, &errorDetailMsg);
	mCount++;
	
	ALOGTEST("startCodec------------6 err:%d input index:%d inputChar addr:%d", err, inputBufferIndex, inputChar);
	
	
	err = mCodec->dequeueOutputBuffer(mEnv, mBufferInfo, &outputBufferIndex, 0);
	ALOGTEST("startCodec------------7 err:%d outputBufferIndex:%d", err, outputBufferIndex);
	
	while (err ==0 && outputBufferIndex >= 0) 
	{
		mCodec->releaseOutputBuffer(outputBufferIndex, true);
		err = mCodec->dequeueOutputBuffer(mEnv, mBufferInfo, &outputBufferIndex, 0);
	}

	if (outputBufferIndex < 0) 
	{
		ALOGTEST("startCodec------------err :%d ", outputBufferIndex);
	}

}


