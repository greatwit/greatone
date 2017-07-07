
#include "FilesDecodec.h"
#include "RtpReceive.h"


#include "ComDefine.h"
#define TAG "FilesDecodec"


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)


char *gFilePath = "/sdcard/camera.h264";

int charsToInt(char* src, int offset) 
{  
	int value;    
	value = (int) ((src[offset]	  & 0xFF)   
				| ((src[offset+1] & 0xFF)<<8)   
				| ((src[offset+2] & 0xFF)<<16)   
				| ((src[offset+3] & 0xFF)<<24));   
	return value;  
} 


FilesDecodec::FilesDecodec()
			:mbRunning(false)
			,mFile(NULL)
			,mCount(0)
{
	ALOGV("SenderServer::SenderServer() construct.");
}

FilesDecodec::~FilesDecodec()
{
	ALOGV("SenderServer, Destructor");
}

bool FilesDecodec::CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	mFile = fopen(gFilePath, "rb");
	mcharLength[4] = {0};
	mData[1000000] = {0};

	mThread = new DecodecThread(this);
	
	mEnv 	= env;
	mCodec 	= new  JMediaCodec(mEnv, thiz, "video/avc", true, false);
	status_t err = mCodec->initCheck();


	#if HEIGHT_VERSION
	    err = mCodec->configure(format, surface->getIGraphicBufferProducer(), crypto, flags);
	#else
	    err = mCodec->configure(format, surface->getSurfaceTexture(), crypto, flags);
	#endif
	
	err = mCodec->start();

    jobjectArray inputBuffers;
	err = mCodec->getBuffers(mEnv, true, &inputBuffers);

	for(int i =0;i<2;i++)
	{
		jobject inputObject = mEnv->GetObjectArrayElement(inputBuffers, i);
		mBufferPoint[i] 	= (uint8_t*)mEnv->GetDirectBufferAddress( inputObject);
	}

	return true;
}

bool FilesDecodec::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	mFile = fopen(gFilePath, "rb");
	mcharLength[4] = {0};
	mData[1000000] = {0};

	mThread = new DecodecThread(this);
	
	mEnv 	= AndroidRuntime::getJNIEnv();
	mCodec 	= new  JMediaCodec(mEnv, NULL, "video/avc", true, false);
	status_t err = mCodec->initCheck();


	#if HEIGHT_VERSION
	    err = mCodec->configure(format, surface->getIGraphicBufferProducer(), crypto, flags);
	#else
	    err = mCodec->configure(format, surface->getSurfaceTexture(), crypto, flags);
	#endif
	
	err = mCodec->start();

    jobjectArray inputBuffers;
	err = mCodec->getBuffers(mEnv, true, &inputBuffers);

	for(int i =0;i<2;i++)
	{
		jobject inputObject = mEnv->GetObjectArrayElement(inputBuffers, i);
		mBufferPoint[i] 	= (uint8_t*)mEnv->GetDirectBufferAddress( inputObject);
	}

	return true;
}

bool FilesDecodec::DeInit()
{	
	if(mbRunning)
		StopVideo();
	
	fclose(mFile);
	
	mCodec->stop();
	mCodec = NULL;
	
	return true;
}


bool FilesDecodec::StartVideo(int deivceid)
{
	if(mbRunning)
		return false;

	mThread->startThread();
	
	run("SenderServer", PRIORITY_URGENT_DISPLAY);
	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);

	mbRunning = true;

	return true;
}

bool FilesDecodec::StopVideo()
{
	if(false==mbRunning)
		return false;

	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	mbRunning = false;
	requestExit();

	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void FilesDecodec::GetDecodecSource()
{
	if(mCount>0)
		mCount--;
	else
		return;
	
	usleep(50*1000);
	
	size_t outputBufferIndex = 0;
	status_t err = mCodec->dequeueOutputBuffer(mEnv, mBufferInfo, &outputBufferIndex, 0);
	ALOGTEST("startCodec------------7 err:%d outputBufferIndex:%d", err, outputBufferIndex);
	
	while (err ==0 && outputBufferIndex >= 0) 
	{
		#if HEIGHT_VERSION
		    mCodec->releaseOutputBuffer(outputBufferIndex, true, false, 0);
		#else
		    mCodec->releaseOutputBuffer(outputBufferIndex, true);
		#endif
		err = mCodec->dequeueOutputBuffer(mEnv, mBufferInfo, &outputBufferIndex, 0);
	}

	if (outputBufferIndex < 0) 
	{
		ALOGTEST("startCodec------------err :%d ", outputBufferIndex);
	}
	
	ALOGTEST("startCodec------------8");
	
}

void FilesDecodec::decorder(char*data, int dataLen)
{
	ALOGTEST("startCodec------------0");
	
	size_t inputBufferIndex = 0;
	
	status_t err;
	
	AString errorDetailMsg;
	
	err = mCodec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("startCodec------------4 err:%d", err);


	uint8_t* inputChar = mBufferPoint[inputBufferIndex];
	memcpy(inputChar, data, dataLen);
	
	err = mCodec->queueInputBuffer(inputBufferIndex, 0, dataLen, mCount * 1000000 / 20, 0, &errorDetailMsg);
	mCount++;
	
	ALOGTEST("startCodec------------6 err:%d input index:%d inputChar addr:%d", err, inputBufferIndex, inputChar);
	
}

bool FilesDecodec::threadLoop()
{
	if(!mbRunning)
		return true;

	int res = 0, dataLen = 0;
	
	res = fread(mcharLength, 4, 1, mFile);
	if(res>0)
	{
		dataLen = charsToInt(mcharLength,0);
		res 	= fread(mData, dataLen, 1, mFile);
		
		VIDEOLOGD("startCodec------------3 res:%d dataLen:%d", res, dataLen);
		
		decorder(mData, dataLen);

		usleep(50*1000);
	}

	return true;
}


