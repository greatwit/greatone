
#include "VideoSender.h"
#include "RtpReceive.h"


#include "ComDefine.h"
#define TAG "VideoSender"

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


VideoSender::VideoSender()
			:mpSender(NULL)
			,mVideoWidth(PixelArray[VIDEO_PIXEL][0])
			,mVideoHeight(PixelArray[VIDEO_PIXEL][1])
			,mbInited(false)
			,mbRunning(false)
			,VideoBase()
			,mFile(NULL)
{

	ALOGV("SenderServer::SenderServer() construct.");
}

VideoSender::~VideoSender()
{
	ALOGV("SenderServer, Destructor");
}

bool VideoSender::Init(ANativeWindow* window, int widht, int height, short sendPort)
{
	if(mbInited)
		return mbInited;

	mVideoWidth = widht;
	mVideoHeight = height;

	mFile = fopen(gFilePath, "rb");
	mcharLength[4] = {0};
	mData[1000000] = {0};

	mpSender = new RtpSender();
	if(!mpSender->initSession(sendPort))
	{
		SAFE_DELETE(mpSender);
		ALOGE("TAG 2,function %s,line:%d mpSender->initSession() failed.", __FUNCTION__, __LINE__);
		return false;
	}

	mbInited = true;

	return true;
}

bool VideoSender::DeInit()
{	
	if(!mbInited)
		return mbInited;
	
	mbInited = false;

	if(mbRunning)
		StopVideo();

	mpSender->deinitSession();
	delete mpSender;
	mpSender = NULL;
	
	fclose(mFile);
	
	return true;
}

bool VideoSender::ChangePixel(int width, int height)
{
	return true;
}

bool VideoSender::ConnectDest(std::string ip, short port)
{
	bool bRes = false;

	if( !mbInited )
		return false;

	bRes = mpSender->connect(ip, port);
	if (bRes == false)
	{
		ALOGE("TAG 2,function %s,line:%d connect device failed... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}
	else
	{
		//mNetInfo.insert(make_pair(ip, NetInfo(ip, port))); 
		ALOGD("TAG 2,function %s,line:%d connect device successfuled... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}

	return bRes;
}

bool VideoSender::IsInited()
{
	return mbInited;
}

bool VideoSender::IsRunning()
{
	return mbRunning;
}

bool VideoSender::StartVideo(int deivceid)
{
	if(mbRunning)
		return false;


	VIDEOLOGD("TAG 1,function %s,line:%d start width:%d height:%d ", __FUNCTION__, __LINE__, mVideoWidth, mVideoHeight);
	run("SenderServer", PRIORITY_URGENT_DISPLAY);
	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);

	mbRunning = true;

	return true;
}

bool VideoSender::StopVideo()
{
		if(false==mbRunning)
			return false;

		ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

		mbRunning = false;
		requestExit();

		VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void VideoSender::RegisterRecorder(RecordBase*record)
{
	mpRecorder = record;
	mbFirstWrite = true;
}

bool VideoSender::isFirstFrame()
{
	return mFirstFrame;
}

void VideoSender::setFirstFrame(bool bFirst)
{
	mFirstFrame = bFirst;
}

bool VideoSender::sendBuffer(void*buff, int dataLen, char*hdrextdata, int numhdrextwords, int64_t timeStamp)
{
	if(mbInited)
	{
		mpSender->sendBuffer(buff, dataLen, hdrextdata, numhdrextwords, timeStamp);
	}
	return false;
}

void VideoSender::registerCallback(IReceiveCallback *base)
{
	mStreamBase = base;
}

bool VideoSender::threadLoop()
{
	if(!mbRunning)
		return true;

	int res = 0, dataLen = 0;
	
	res = fread(mcharLength, 4, 1, mFile);
	if(res>0)
	{
		dataLen = charsToInt(mcharLength,0);
		res = fread(mData, dataLen, 1, mFile);
		
		VIDEOLOGD("startCodec------------3 res:%d dataLen:%d", res, dataLen);
		
		//sendBuffer(mData, dataLen, (char*)"h264", 5, 0);
		
		if(mStreamBase!=NULL)
		{
				mStreamBase->ReceiveSource(0, (char*)"", (void*)mData, dataLen);
		}
		
		usleep(50*1000);
	}

	return true;
}


