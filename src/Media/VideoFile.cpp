

#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>



#include "VideoFile.h"

#include "ComDefine.h"
#define	 TAG "VideoFile"

#define filepath "/sdcard/avctest.h264"

using namespace android;

char marker[4] = {0, 0, 0, 1};
int frameMatch(char*marker, char*buffer, int start ,int remain)
{
	int j = 0, i =0;
	for(i = start; i<remain; i++)
	{
		if (buffer[i] == marker[j])
		{
			j++;
			if (j == 4)
				return i - (j - 1);
		}
		else
			j = 0;
	}
	return -1;
}

VideoFile::VideoFile()
		:mRunning(false)
		,VideoBase()
		,mFile(NULL)
{
	mCodec 	= new CodecBase("video/avc", true, false);
	
	mThread = new FileProcessThread(this); 
	mFile 	= fopen(filepath, "rb");
	if(mFile)
	{
		long length = 0;
		fseek(mFile, 0, SEEK_END);
		length = ftell(mFile);
		fseek(mFile, 0, SEEK_SET);
		
		mFileBuffer = new char[length];
		fread(mFileBuffer, sizeof(char), length, mFile);
		
		mStartIndex = 0;
		mFileLen	= length;
	}
	 
	ALOGV("VideoFile construct.");
}

VideoFile::~VideoFile() 
{
	mCodec = NULL;
	ALOGD("TAG ,function %s,line:%d VideoFile destroyed.",__FUNCTION__,__LINE__);
}

bool VideoFile::Init(sp<AMessage> &format, sp<Surface> &surface, sp<ICrypto> &crypto, int flags) 
{
	ALOGV("Enter:init VideoFile----------->");
    
	mCodec->CreateCodec(format, surface, crypto, flags);


	return true;
}//end init

bool VideoFile::DeInit()
{
	mCodec 	= NULL;
	mThread = NULL;
	
	if(mFile)
	{
		fclose(mFile);
		mFile = NULL;
		delete mFileBuffer;
		mFileBuffer = NULL;
	}

	
	return true;
}

bool VideoFile::IsInited()
{
	return mbInited;
}

bool VideoFile::StartVideo(int cardid)
{
	bool bRest = true;

	ALOGD("function %s,line:%d VideoFile::StartVideo() successful 1.",__FUNCTION__,__LINE__);
	
	mCodec->startCodec();

	if(false == mRunning)
	{
		mRunning = true;
		ALOGV("TAG 1,function %s,line:%d",__FUNCTION__,__LINE__);
		run("VideoFile", PRIORITY_URGENT_DISPLAY);
		ALOGV("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);
	}
	
	mThread->startThread();

	ALOGE("function %s,line:%d VideoFile::StartVideo() successful 2.",__FUNCTION__,__LINE__);

	return bRest;
}

bool VideoFile::StopVideo()
{
	bool bRest = true;
	ALOGD("function %s,line:%d TalkbackFinish VideoFile StopVideo...1", __FUNCTION__,__LINE__);
	
	mThread->stopThread();
	
	mRunning = false;
	requestExit();
	
	mCodec->stopCodec();

	ALOGD("function %s,line:%d TalkbackFinish VideoFile StopVideo...2", __FUNCTION__,__LINE__);

	return bRest;
}

void VideoFile::RegisterRecorder(RecordBase*record)
{
	mpRecorder		= record;
}

bool VideoFile::isFirstFrame()
{
	return mbFirstFrame;
}

void VideoFile::setFirstFrame(bool bFirst)
{
	mbFirstFrame = bFirst;
}

void VideoFile::GetFileSource()
{
	if (mFileLen == 0 || mStartIndex >= mFileLen) 
	{
		return;
	}
	int nextFrameStart = frameMatch(marker, mFileBuffer, mStartIndex+2, mFileLen);
	if (nextFrameStart == -1) 
	{
		nextFrameStart = mFileLen;
	} else 
	{
	}
	//ALOGD("function %s,line:%d frame size:%d\n", __FUNCTION__,__LINE__, nextFrameStart - mStartIndex);
	
	mCodec->addBuffer(mFileBuffer+mStartIndex, nextFrameStart - mStartIndex);
	
	mStartIndex = nextFrameStart;
	
	usleep(50000);
}

bool VideoFile::threadLoop()
{
	//ALOGD("function %s,line:%d threadLoop------------------------", __FUNCTION__,__LINE__);
	//mCodec->getCodecBuffer();
	return true;
}

bool VideoFile::setBuffer(void* data,int len, int64_t timeStamp)
{
	mCodec->addBuffer((char*)data, len);
	return true;
}

void VideoFile::ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen)
{
	ALOGD("function:%s,line=:%d, data length=:%d mimetype:%s ",__FUNCTION__,__LINE__, dataLen, mimeType);
	//if(0 == strcmp(mimeType, MIME_H264))
	{
		setBuffer(buffer, dataLen, timeStamp);
	}
	
	return;
}



