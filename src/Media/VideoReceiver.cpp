
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>
#include <gui/Surface.h>

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

		ALOGV("VideoReceiver construct.");
	}

VideoReceiver::~VideoReceiver() 
{
	ALOGD("TAG ,function %s,line:%d VideoReceiver destroyed.",__FUNCTION__,__LINE__);
}

bool VideoReceiver::Init(ANativeWindow *pNativeWindow, int nWidth, int nHeight, short recvPort) 
{
	ALOGV("Enter:init VideoReceiver----------->");

	if(mbInited)
		return mbInited;


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
	
	if(!mbInited)
		return mbInited;
	

	ALOGD("TAG 2,function %s,line:%d startVideoReceive Width:%d,Height:%d", __FUNCTION__, __LINE__, miPixelWidth, miPixelHeight);


	mpReceive->startThread();

	if(false == mRunning)
	{
		mRunning = true;
		ALOGV("TAG 1,function %s,line:%d",__FUNCTION__,__LINE__);
		run("VideoReceiver", PRIORITY_URGENT_DISPLAY);
		ALOGV("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);
	}

	ALOGV("LEAVE:VideoReceiver::StartVideo() successful.,function %s,line:%d",__FUNCTION__,__LINE__);

	return bRest;
}

bool VideoReceiver::StopVideo()
{
	bool bRest = true;
	ALOGD("function %s,line:%d TalkbackFinish VideoReceiver StopVideo...1", __FUNCTION__,__LINE__);

	mRunning = false;

	mpReceive->stopThread();

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
		usleep(20000);
	
		return true;
}


bool VideoReceiver::setBuffer(void* data,int len, int64_t timeStamp)
{

	return true;
}

void VideoReceiver::ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen)
{
	if(0 == strcmp(mimeType, MIME_H264))
	{
		setBuffer(buffer, dataLen, timeStamp);
	}
	//ALOGD("TAG 4:%s,line=:%d, data length=:%d mimetype:%s ",__FUNCTION__,__LINE__, dataLen, mimeType);

	return;
}



