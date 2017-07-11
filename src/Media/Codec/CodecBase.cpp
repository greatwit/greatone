
#include "CodecBase.h"

#include "media_MediaCodec.h"

#include "Configure.h"

#define TAG "CodecBase"


namespace android 
{

////////////////////////////////////////////////////////////////////////////////

CodecBase::CodecBase(const char *name, bool nameIsType, bool encoder)
			:mbRunning(false)
			,mpInputBufferPoint(NULL)
			,mpOutputBufferPoint(NULL)
			,mCount(0)
{
	mpEnv = AndroidRuntime::getJNIEnv();

	mCodec = new JMediaCodec(mpEnv, NULL, name, nameIsType, encoder);
	
	pthread_mutex_init(&mMutex,NULL);
	mCond  = PTHREAD_COND_INITIALIZER;
}


CodecBase::~CodecBase() 
{
    if (mCodec != NULL) 
	{
        mCodec = NULL;
    }
	
	pthread_mutex_destroy(&mMutex);
	pthread_cond_destroy(&mCond);
}

status_t CodecBase::CreateCodec(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	status_t err;

	ALOGTEST("function %s,line:%d configure...1", __FUNCTION__, __LINE__);
	
	#if HEIGHT_VERSION
		err = mCodec->configure(format, surface->getIGraphicBufferProducer(), crypto, flags);
		ALOGTEST("function %s,line:%d configure...2 err:%d", __FUNCTION__, __LINE__, err);
	#else
		err = mCodec->configure(format, surface->getSurfaceTexture(), crypto, flags);
		ALOGTEST("function %s,line:%d configure...2 err:%d", __FUNCTION__, __LINE__, err);
	#endif

	return err;
}

status_t CodecBase::startCodec()
{
	status_t err;
	err = mCodec->start();
	
	jobjectArray inputArray, outputArray;
	
	err = mCodec->getBuffers(mpEnv, true, &inputArray); 	//input true, output false
	ALOGTEST("startCodec------------3 err:%d", err);
	
	if(OK == err)
	{
		miInputArrayLen = mpEnv->GetArrayLength(inputArray);
		ALOGTEST("startCodec------------4 miInputArrayLen:%d", miInputArrayLen);
		mpInputBufferPoint = new int[miInputArrayLen];
		for(int i=0; i<miInputArrayLen; i++)
		{
			jobject buffer 			= mpEnv->GetObjectArrayElement(inputArray, i);
			mpInputBufferPoint[i] 	= (int)mpEnv->GetDirectBufferAddress(buffer);
			ALOGTEST("startCodec------------BufferAddress:%d", mpInputBufferPoint[i]);
		}
	}

	err = mCodec->getBuffers(mpEnv, false, &outputArray); 	//input true, output false
	ALOGTEST("startCodec------------5 err:%d", err);
	
	if(OK == err)
	{
		miOutputArrayLen = mpEnv->GetArrayLength(outputArray);
		ALOGTEST("startCodec------------6 miOutputArrayLen:%d", miOutputArrayLen);
		mpOutputBufferPoint = new int[miOutputArrayLen];
		for(int i=0; i<miOutputArrayLen; i++)
		{
			jobject buffer 			= mpEnv->GetObjectArrayElement(outputArray, i);
			mpOutputBufferPoint[i] 	= (int)mpEnv->GetDirectBufferAddress(buffer);
			ALOGTEST("startCodec------------BufferAddress:%d", mpOutputBufferPoint[i]);
		}
	}

	return err;
}

status_t CodecBase::stopCodec()
{
	status_t err;
	err = mCodec->stop();
	
	if(mpInputBufferPoint)
	{
		delete mpInputBufferPoint;
		mpInputBufferPoint = NULL;
	}
	
	if(mpOutputBufferPoint)
	{
		delete mpOutputBufferPoint;
		mpOutputBufferPoint = NULL;
	}
	
	return err;
}

status_t CodecBase::addBuffer(char*data, int len)
{
	status_t err;
	size_t inputBufferIndex = 0, outputBufferIndex = 0;
	AString errorDetailMsg;
	
	//pthread_mutex_lock(&mMutex);

	err = mCodec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("addBuffer------------1 err:%d", err);

	uint8_t* inputChar = (uint8_t*)mpInputBufferPoint[inputBufferIndex];
	memcpy(inputChar, data, len);
	
	err = mCodec->queueInputBuffer(inputBufferIndex, 0, len, 0 * 1000000 / 20, 0, &errorDetailMsg);
	ALOGTEST("addBuffer------------2 err:%d", err);
	
	mCount++;
	
	//pthread_mutex_unlock(&mMutex);
	//pthread_cond_signal(&mCond);
	
	return err;
}

status_t CodecBase::getCodecBuffer()
{
	status_t err;
	size_t outputBufferIndex = 0;
	
	if(mCount>0)
		mCount--;
	else
		return -1;
	
	//pthread_mutex_lock(&mMutex);
	//pthread_cond_wait(&mCond, &mMutex);
		
	//usleep(20*1000);
		
	err = mCodec->dequeueOutputBuffer(mpEnv, NULL, &outputBufferIndex, 0);
	ALOGTEST("getCodecBuffer------------7 err:%d outputBufferIndex:%d", err, outputBufferIndex);
	while (err ==0 && outputBufferIndex >= 0) 
	{
		#if HEIGHT_VERSION
			mCodec->releaseOutputBuffer(outputBufferIndex, true, false, 0);
		#else
			mCodec->releaseOutputBuffer(outputBufferIndex, true);
		#endif
		err = mCodec->dequeueOutputBuffer(mpEnv, NULL, &outputBufferIndex, 0);
	}

	if (outputBufferIndex < 0) 
	{
		ALOGTEST("getCodecBuffer------------err :%d ", outputBufferIndex);
	}
	
	//pthread_mutex_unlock(&mMutex);
	ALOGTEST("getCodecBuffer------------8");

	return err;
}


}  // namespace android

////////////////////////////////////////////////////////////////////////////////