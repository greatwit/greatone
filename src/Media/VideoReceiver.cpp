

#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>
#include <gui/Surface.h>


#include <media/ICrypto.h>

#include "VideoReceiver.h"

#include "ComDefine.h"
#define	 TAG "VideoReceiver"



using namespace android;


VideoReceiver::VideoReceiver()
		:mRunning(false)
		,mbInited(false)
		,VideoBase()
{
	mbFirstFrame = true;

	mCodec = new CodecBase( "video/avc", true, false);//nameIsType, encoder
	
	ALOGV("VideoReceiver construct.");
}

VideoReceiver::~VideoReceiver() 
{
	mCodec = NULL;
	ALOGD("TAG ,function %s,line:%d VideoReceiver destroyed.",__FUNCTION__,__LINE__);
}

bool VideoReceiver::Init(sp<AMessage> &format, sp<Surface> &surface, sp<ICrypto> &crypto, int flags, short recvPort) 
{
	ALOGV("Enter:init VideoReceiver----------->");
    
	if(mbInited)
		return mbInited;

	status_t err;
	
	mCodec->CreateCodec(format, surface, crypto, flags);

	mpReceive = new RtpReceive();
	if(mpReceive->initSession(recvPort))
	{
		mpReceive->registerCallback(this);
	}
	else
	{
		ALOGE("TAG 2,function %s,line:%d  RtpReceive::Singleton->initSession failed",__FUNCTION__,__LINE__);
		//return false;
	}

	ALOGV("TAG 1:%s,line:%d",__FUNCTION__,__LINE__);


	mbInited = true;

	return true;
}//end init

bool VideoReceiver::DeInit()
{
	if(!mbInited)
		return mbInited;

	mbFirstFrame = true;
	ALOGD("function %s,line:%d TalkbackFinish VideoReceiver deInit...1", __FUNCTION__,__LINE__);

	mpReceive->deinitSession();
	SAFE_DELETE(mpReceive);

	ALOGD("function %s,line:%d TalkbackFinish VideoReceiver deInit...2", __FUNCTION__,__LINE__);

	mbInited = false;
	
	return true;
}

bool VideoReceiver::IsInited()
{
	return mbInited;
}

bool VideoReceiver::StartVideo(int cardid)
{
	bool bRest = true;

	ALOGD("function %s,line:%d VideoReceiver::StartVideo() successful 1.",__FUNCTION__,__LINE__);
	

	if(false == mRunning)
	{
		mRunning = true;
		ALOGV("TAG 1,function %s,line:%d",__FUNCTION__,__LINE__);
		run("VideoReceiver", PRIORITY_URGENT_DISPLAY);
		ALOGV("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);
	}
	
	mpReceive->startThread();

	ALOGE("function %s,line:%d VideoReceiver::StartVideo() successful 2.",__FUNCTION__,__LINE__);

	return bRest;
}

bool VideoReceiver::StopVideo()
{
	bool bRest = true;
	ALOGD("function %s,line:%d TalkbackFinish VideoReceiver StopVideo...1", __FUNCTION__,__LINE__);

	mpReceive->stopThread();
	
	mCodec->stopCodec();
	
	mRunning = false;
	requestExit();

	ALOGD("function %s,line:%d TalkbackFinish VideoReceiver StopVideo...2", __FUNCTION__,__LINE__);

	return bRest;
}

void VideoReceiver::RegisterRecorder(RecordBase*record)
{
	mpRecorder		= record;
	mbFirstWrite	= true;
}

bool VideoReceiver::isFirstFrame()
{
	return mbFirstFrame;
}

void VideoReceiver::setFirstFrame(bool bFirst)
{
	mbFirstFrame = bFirst;
}

bool VideoReceiver::threadLoop()
{
	ALOGD("function %s,line:%d threadLoop------------------------", __FUNCTION__,__LINE__);
	mCodec->startCodec();
	return true;
}

bool VideoReceiver::setBuffer(void* data,int len, int64_t timeStamp)
{
	mCodec->addBuffer((char*)data, len);
	return true;
}

void VideoReceiver::ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen)
{
	ALOGD("function:%s,line=:%d, data length=:%d mimetype:%s ",__FUNCTION__,__LINE__, dataLen, mimeType);
	//if(0 == strcmp(mimeType, MIME_H264))
	{
		setBuffer(buffer, dataLen, timeStamp);
	}
	

	return;
}



