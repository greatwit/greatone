

#define LOG_TAG "GMediaCodec-JNI"
#include <utils/Log.h>

#include "GMediaCodec.h"
#include "media_Utils.h"

#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "jni.h"
#include "JNIHelp.h"


#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>

#include <system/window.h>

#define TAG "GMediaCodec"

#define HEIGHT_VERSION false

namespace android 
{

////////////////////////////////////////////////////////////////////////////////

GMediaCodec::GMediaCodec(const char *name, bool nameIsType, bool encoder)
			:mClass(NULL)
			,mObject(NULL)
			,mbRunning(false)
{
	mpEnv = AndroidRuntime::getJNIEnv();

    mLooper = new ALooper;
    mLooper->setName("MediaCodec_looper");

    mLooper->start(
            false,      // runOnCallingThread
            false,       // canCallJava
            PRIORITY_DEFAULT);

    if (nameIsType) 
	{
        mCodec = MediaCodec::CreateByType(mLooper, name, encoder);
    } 
	else 
	{
        mCodec = MediaCodec::CreateByComponentName(mLooper, name);
    }
	
	pthread_mutex_init(&mMutex,NULL);
	mCond  = PTHREAD_COND_INITIALIZER;
}

status_t GMediaCodec::initCheck() const 
{
    return mCodec != NULL ? OK : NO_INIT;
}

GMediaCodec::~GMediaCodec() 
{
    if (mCodec != NULL) 
	{
        mCodec->release();
        mCodec.clear();
    }
	
	pthread_mutex_destroy(&mMutex);
	pthread_cond_destroy(&mCond);
}

status_t GMediaCodec::configure(
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

status_t GMediaCodec::start() 
{
	status_t err;
	err = mCodec->start();
	jobjectArray inputBuffers;
	err = getBuffers(true, &inputBuffers);
	return err;
}

status_t GMediaCodec::stop() 
{
    mSurfaceTextureClient.clear();
    return mCodec->stop();
}

status_t GMediaCodec::flush() 
{
    return mCodec->flush();
}

status_t GMediaCodec::queueInputBuffer( size_t index, size_t offset, size_t size, int64_t timeUs, uint32_t flags, AString *errorDetailMsg) 
{
    return mCodec->queueInputBuffer( index, offset, size, timeUs, flags, errorDetailMsg);
}


status_t GMediaCodec::dequeueInputBuffer(size_t *index, int64_t timeoutUs) 
{
    return mCodec->dequeueInputBuffer(index, timeoutUs);
}

status_t GMediaCodec::dequeueOutputBuffer(jobject bufferInfo, size_t *index, int64_t timeoutUs) 
{
    size_t size, offset;
    int64_t timeUs;
    uint32_t flags;
    status_t err;
    if ((err = mCodec->dequeueOutputBuffer( index, &offset, &size, &timeUs, &flags, timeoutUs)) != OK) 
	{
        return err;
    }

    jclass clazz = mpEnv->FindClass("android/media/MediaCodec$BufferInfo");

    jmethodID method = mpEnv->GetMethodID(clazz, "set", "(IIJI)V");
    mpEnv->CallVoidMethod(bufferInfo, method, offset, size, timeUs, flags);

    return OK;
}

status_t GMediaCodec::dequeueOutputBuffer( size_t *index, int64_t timeoutUs)
{
	size_t size, offset;
    int64_t timeUs;
    uint32_t flags;
    status_t err;
    if ((err = mCodec->dequeueOutputBuffer( index, &offset, &size, &timeUs, &flags, timeoutUs)) != OK) 
	{
        return err;
    }

    return OK;
}

status_t GMediaCodec::releaseOutputBuffer(size_t index, bool render) 
{
    return render
        ? mCodec->renderOutputBufferAndRelease(index)
        : mCodec->releaseOutputBuffer(index);
}

status_t GMediaCodec::getOutputFormat(jobject *format) const 
{
    sp<AMessage> msg;
    status_t err;
    if ((err = mCodec->getOutputFormat(&msg)) != OK) { 
        return err;
    }

    return ConvertMessageToMap(mpEnv, msg, format);
}

status_t GMediaCodec::getBuffers(bool input, jobjectArray *bufArray) 
{
    Vector<sp<ABuffer> > buffers;

    status_t err = input ? mCodec->getInputBuffers(&buffers) : mCodec->getOutputBuffers(&buffers);

    if (err != OK) {
        return err;
    }

    for (size_t i = 0; i < buffers.size(); ++i) 
	{
        const sp<ABuffer> &buffer = buffers.itemAt(i);

        jobject byteBuffer = mpEnv->NewDirectByteBuffer( buffer->base(), buffer->capacity());
		mpBufferPoint[i] 	=  (uint8_t*)mpEnv->GetDirectBufferAddress( byteBuffer);

        mpEnv->DeleteLocalRef(byteBuffer);
        byteBuffer = NULL;
    }

    return OK;
}

status_t GMediaCodec::addBuffer(char*data, int len)
{
	size_t inputBufferIndex = 0, outputBufferIndex = 0;
	
	status_t err;
	AString errorDetailMsg;
	
	pthread_mutex_lock(&mMutex);

	err = mCodec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("decorder------------4 err:%d", err);

	uint8_t* inputChar = mpBufferPoint[inputBufferIndex];
	memcpy(inputChar, data, len);
	
	err = mCodec->queueInputBuffer(inputBufferIndex, 0, len, 0 * 1000000 / 20, 0, &errorDetailMsg);
	pthread_mutex_unlock(&mMutex);
	pthread_cond_signal(&mCond);
	
	ALOGTEST("decorder------------6 err:%d input index:%d inputChar addr:%d", err, inputBufferIndex, inputChar);
	return OK;
}

status_t GMediaCodec::startCodec()
{
	size_t outputBufferIndex = 0;
	status_t err;
	
	err = start();
	mbRunning = true;
	while (mbRunning)
	{
			pthread_mutex_lock(&mMutex);
			pthread_cond_wait(&mCond, &mMutex);
		    
			err = dequeueOutputBuffer(&outputBufferIndex, 0);
			ALOGTEST("startCodec------------7 err:%d outputBufferIndex:%d", err, outputBufferIndex);
			while (err ==0 && outputBufferIndex >= 0) 
			{
				#if HEIGHT_VERSION
					releaseOutputBuffer(outputBufferIndex, true, false, 0);
				#else
					releaseOutputBuffer(outputBufferIndex, true);
				#endif
				err = dequeueOutputBuffer(&outputBufferIndex, 0);
			}

			if (outputBufferIndex < 0) 
			{
				ALOGTEST("startCodec------------err :%d ", outputBufferIndex);
			}
			
			pthread_mutex_unlock(&mMutex);
			ALOGTEST("startCodec------------8");
			usleep(50*1000);
	} 
	
	return OK;
}

status_t GMediaCodec::stopCodec()
{
	mbRunning = false;
	stop();
	return OK;
}

}  // namespace android

////////////////////////////////////////////////////////////////////////////////
