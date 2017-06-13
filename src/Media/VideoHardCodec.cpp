
#include "VideoHardCodec.h"



#include "android_media_MediaCrypto.h"
#include "android_media_Utils.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"

#include <gui/Surface.h>
#include <media/ICrypto.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaCodec.h>

#if HEIGHT_VERSION
#else
#include <gui/SurfaceTextureClient.h>
#endif

#define TAG "VideoHardCodec"



/*
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
*/

namespace android {

VideoHardCodec::VideoHardCodec(JNIEnv *env, jobject thiz)
				: mClass(NULL)
				,mObject(NULL) 
{
    jclass clazz = env->GetObjectClass(thiz);
    CHECK(clazz != NULL);

    mClass = (jclass)env->NewGlobalRef(clazz);
    mObject = env->NewWeakGlobalRef(thiz);

	mLooper = new ALooper;
    mLooper->setName("MediaCodec_looper");
    mLooper->start(
            false,      // runOnCallingThread
            false,       // canCallJava
            PRIORITY_DEFAULT);
	
	mEnv  = env;
   	mThiz = thiz;
}

VideoHardCodec::~VideoHardCodec()
{
    if (mCodec != NULL) {
        mCodec->release();
        mCodec.clear();
    }

    JNIEnv *env = AndroidRuntime::getJNIEnv();

    env->DeleteWeakGlobalRef(mObject);
    mObject = NULL;
    env->DeleteGlobalRef(mClass);
    mClass = NULL;
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

    if (true)
	{
        mCodec = MediaCodec::CreateByType(mLooper, tmp, false); //encode
    } else {
        mCodec = MediaCodec::CreateByComponentName(mLooper, tmp);
    }

    status_t err ;//= mCodec->initCheck();

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
	/*
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
	*/
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

	    err = configure(format, surfaceTexture, crypto, flags);
	#endif

        //throwExceptionAsNecessary(mEnv, err);
}

status_t VideoHardCodec::configure(
        const sp<AMessage> &format,
        const sp<ISurfaceTexture> &surfaceTexture,
        const sp<ICrypto> &crypto,
        int flags) 
{
    sp<SurfaceTextureClient> client;
    if (surfaceTexture != NULL) {
        mSurfaceTextureClient = new SurfaceTextureClient(surfaceTexture);
    } else {
        mSurfaceTextureClient.clear();
    }

    return mCodec->configure(format, mSurfaceTextureClient, crypto, flags);
}

void VideoHardCodec::Finished()
{
	mCodec->decStrong(mThiz);
}

void VideoHardCodec::Start(jobject bufferInfo)
{
    status_t err = mCodec->start();
	
	jobjectArray inputBuffers;
	err = getBuffers(mEnv, true, &inputBuffers);

	for(int i =0;i<2;i++)
	{
		jobject inputObject 	= mEnv->GetObjectArrayElement(inputBuffers, i);
		mBufferPoint[i] 	= (uint8_t*)mEnv->GetDirectBufferAddress( inputObject);
	}
	
	mBufferInfo = bufferInfo;
	
	size_t outputBufferIndex = 0;
	do 
	{
		usleep(50*1000);
		err = dequeueOutputBuffer(mEnv, mBufferInfo, &outputBufferIndex, 0);
		ALOGTEST("startCodec------------7 err:%d outputBufferIndex:%d", err, outputBufferIndex);
		while (err ==0 && outputBufferIndex >= 0) 
		{
			#if HEIGHT_VERSION
				releaseOutputBuffer(outputBufferIndex, true, false, 0);
			#else
				releaseOutputBuffer(outputBufferIndex, true);
			#endif
			err = dequeueOutputBuffer(mEnv, mBufferInfo, &outputBufferIndex, 0);
		}

		if (outputBufferIndex < 0) 
		{
			ALOGTEST("startCodec------------err :%d ", outputBufferIndex);
		}
		
		ALOGTEST("startCodec------------8");
	} while (true);
	
	/*
    throwExceptionAsNecessary(mEnv, err);
	*/
}

void VideoHardCodec::Stop()
{
	mSurfaceTextureClient.clear();
    status_t err = mCodec->stop();
}

status_t VideoHardCodec::dequeueOutputBuffer( JNIEnv *env, jobject bufferInfo, size_t *index, int64_t timeoutUs) 
{
    size_t size, offset;
    int64_t timeUs;
    uint32_t flags;
    status_t err;
    if ((err = mCodec->dequeueOutputBuffer(
                    index, &offset, &size, &timeUs, &flags, timeoutUs)) != OK) {
        return err;
    }

    jclass clazz = env->FindClass("android/media/MediaCodec$BufferInfo");

    jmethodID method = env->GetMethodID(clazz, "set", "(IIJI)V");
    env->CallVoidMethod(bufferInfo, method, offset, size, timeUs, flags);

    return OK;
}

status_t VideoHardCodec::dequeueOutputBuffer( JNIEnv *env, size_t *index, int64_t timeoutUs)
{
	size_t size, offset;
    int64_t timeUs;
    uint32_t flags;
    status_t err;
    if ((err = mCodec->dequeueOutputBuffer( index, &offset, &size, &timeUs, &flags, timeoutUs)) != OK) {
        return err;
    }

    return OK;
}

status_t VideoHardCodec::releaseOutputBuffer(size_t index, bool render) 
{
    return render ? mCodec->renderOutputBufferAndRelease(index) : mCodec->releaseOutputBuffer(index);
}

status_t VideoHardCodec::getBuffers( JNIEnv *env, bool input, jobjectArray *bufArray) const 
{
    Vector<sp<ABuffer> > buffers;

    status_t err = input ? mCodec->getInputBuffers(&buffers) : mCodec->getOutputBuffers(&buffers);

    if (err != OK) {
        return err;
    }

    jclass byteBufferClass = env->FindClass("java/nio/ByteBuffer");
    CHECK(byteBufferClass != NULL);

    jmethodID orderID = env->GetMethodID(
            byteBufferClass,
            "order",
            "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");

    CHECK(orderID != NULL);

    jclass byteOrderClass = env->FindClass("java/nio/ByteOrder");
    CHECK(byteOrderClass != NULL);

    jmethodID nativeOrderID = env->GetStaticMethodID(
            byteOrderClass, "nativeOrder", "()Ljava/nio/ByteOrder;");
    CHECK(nativeOrderID != NULL);

    jobject nativeByteOrderObj =
        env->CallStaticObjectMethod(byteOrderClass, nativeOrderID);
    CHECK(nativeByteOrderObj != NULL);

    *bufArray = (jobjectArray)env->NewObjectArray(
            buffers.size(), byteBufferClass, NULL);

    for (size_t i = 0; i < buffers.size(); ++i) {
        const sp<ABuffer> &buffer = buffers.itemAt(i);

        jobject byteBuffer =
            env->NewDirectByteBuffer(
                buffer->base(),
                buffer->capacity());

        jobject me = env->CallObjectMethod(
                byteBuffer, orderID, nativeByteOrderObj);
        env->DeleteLocalRef(me);
        me = NULL;

        env->SetObjectArrayElement(
                *bufArray, i, byteBuffer);

        env->DeleteLocalRef(byteBuffer);
        byteBuffer = NULL;
    }

    env->DeleteLocalRef(nativeByteOrderObj);
    nativeByteOrderObj = NULL;

    return OK;
}

void VideoHardCodec::AddBuffer(char*data, int dataLen)
{
	size_t inputBufferIndex = 0, outputBufferIndex = 0;
	
	status_t err;
	int mCount = 0;
	AString errorDetailMsg;
	
	err = mCodec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("decorder------------4 err:%d", err);

	uint8_t* inputChar = mBufferPoint[inputBufferIndex];
	memcpy(inputChar, data, dataLen);
	
	err = mCodec->queueInputBuffer(inputBufferIndex, 0, dataLen, mCount * 1000000 / 20, 0, &errorDetailMsg);
	mCount++;
	
	ALOGTEST("decorder------------6 err:%d input index:%d inputChar addr:%d", err, inputBufferIndex, inputChar);
}

}


