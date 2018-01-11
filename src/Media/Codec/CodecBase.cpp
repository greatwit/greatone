
#include "CodecBase.h"

#include "media_MediaCodec.h"

#include "Configure.h"

#define TAG "CodecBase"





namespace android 
{

////////////////////////////////////////////////////////////////////////////////

CodecBase::CodecBase(const char *name, bool nameIsType, bool encoder)
			:mpInputBufferPoint(NULL)
			,mpOutputBufferPoint(NULL)
			,mpBufferCall(NULL)
			,mCount(0)
{
	mpEnv = AndroidRuntime::getJNIEnv();

	mCodec = new JMediaCodec(mpEnv, NULL, name, nameIsType, encoder);
}


CodecBase::~CodecBase() 
{
    if (mCodec != NULL) 
	{
        mCodec = NULL;
    }
}

void  	 CodecBase::registerBufferCall(ICodecCallback *call)
{
	mpBufferCall = call;
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
			if(buffer)
			{
				mpInputBufferPoint[i] 	= (int)mpEnv->GetDirectBufferAddress(buffer);
				ALOGTEST("startCodec------------BufferAddress:%d", mpInputBufferPoint[i]);
			}
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
			if(buffer)
			{
				mpOutputBufferPoint[i] 	= (int)mpEnv->GetDirectBufferAddress(buffer);
				ALOGTEST("startCodec------------BufferAddress:%d", mpOutputBufferPoint[i]);
			}
		}

	}

	run("CodecBase", PRIORITY_URGENT_DISPLAY);

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
	
	requestExit();

	return err;
}

status_t CodecBase::addBuffer(char*data, int len)
{
	status_t err;
	size_t inputBufferIndex = 0, outputBufferIndex = 0;
	AString errorDetailMsg;
	
	err = mCodec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("addBuffer 1 err:%d inputBufferIndex:%d", err, inputBufferIndex);

	if(err==0)
	{
		void* inputChar = (void*)mpInputBufferPoint[inputBufferIndex];
		ALOGTEST("addBuffer 2 inputChar:%d", inputChar);
		memcpy(inputChar, data, len);
		ALOGTEST("addBuffer 22 len:%d", len);
		err = mCodec->queueInputBuffer(inputBufferIndex, 0, len, 1000000, 0, &errorDetailMsg);
		ALOGTEST("addBuffer 3 err:%d", err);
	
		mCount++;
	}

	return err;
}

status_t CodecBase::getCodecBuffer()
{
	status_t err;
	size_t outputBufferIndex = 0;
	size_t size, offset;
	int64_t timeUs;
	uint32_t flags;
	
	if(mCount>0)
		mCount--;
	else
		return -1;

	err = mCodec->dequeueOutputBuffer(mpEnv, &outputBufferIndex, offset, size, timeUs, flags, 12000);
	ALOGE("dequeue 7 size:%d uflags:%d err:%d outputBufferIndex:%d", size, flags, err, outputBufferIndex);

	while (err ==0 && outputBufferIndex >= 0) 
	{
		if(mpBufferCall)
		{	
			mBufferINfo.buf = (char*)mpOutputBufferPoint[outputBufferIndex];
			mBufferINfo.size = size;
			mBufferINfo.flags = flags;
			mpBufferCall->onCodecBuffer(mBufferINfo);
		}

		mCodec->releaseOutputBuffer(outputBufferIndex, true, false, 0);

		err = mCodec->dequeueOutputBuffer(mpEnv, &outputBufferIndex, offset, size, timeUs, flags, 12000);
		if(err==0)
			ALOGE("dequeue 8 size:%d uflags:%d", size, flags);
	}

	if (outputBufferIndex < 0) 
	{
		ALOGTEST("getCodecBuffer------------err :%d ", outputBufferIndex);
	}
	
	ALOGTEST("getCodecBuffer------------8");

	return err;
}

bool CodecBase::threadLoop()
{
	getCodecBuffer();
	usleep(50*1000);
	return true;
}

}  // namespace android

////////////////////////////////////////////////////////////////////////////////
