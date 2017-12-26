
#include "FileDeCodec.h"
#include "RtpReceive.h"

#include "CodecApi.h"

#include "ComDefine.h"
#define TAG "FileDeCodec"


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)


//char *gFilePath1 = "/sdcard/camera.h264";

int charsToInt1(char* src, int offset) 
{  
	int value;
	value = (int) ((src[offset]	  & 0xFF)
				| ((src[offset+1] & 0xFF)<<8)
				| ((src[offset+2] & 0xFF)<<16)
				| ((src[offset+3] & 0xFF)<<24));
	return value;
} 


FileDeCodec::FileDeCodec()
			:mbRunning(false)
			,mFile(NULL)
{
	ALOGV("FileDeCodec::FileDeCodec() construct.");
}

FileDeCodec::~FileDeCodec()
{
	ALOGV("FileDeCodec, Destructor");
}


bool FileDeCodec::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, char*filename)
{
	mFile = fopen(filename, "rb");
	mcharLength[4] = {0};
	mData[1000000] = {0};
	
	//mCodec = new CodecBase("video/avc", true, false);
	//mCodec->CreateCodec(format, surface, crypto, flags);
	CodecBaseLib::getInstance()->CodecCreate(format, surface, crypto, flags, false);
	
	return true;
}

bool FileDeCodec::DeInit()
{	
	if(mbRunning)
		StopVideo();
	
	fclose(mFile);
	
	return true;
}


bool FileDeCodec::StartVideo(int deivceid)
{
	if(mbRunning)
		return false;

	//mCodec->startCodec();
	CodecBaseLib::getInstance()->StartCodec();

	
	run("SenderServer", PRIORITY_URGENT_DISPLAY);
	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);

	mbRunning = true;  

	return true;
}

bool FileDeCodec::StopVideo()
{
	if(false==mbRunning)
		return false;

	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	mbRunning = false;
	requestExit();

	//mCodec->stopCodec();
	CodecBaseLib::getInstance()->StopCodec();
	
	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void FileDeCodec::onCodecBuffer(struct CodecBuffer& buff)
{
}


bool FileDeCodec::threadLoop()
{
	if(!mbRunning)
		return true;

	int res = 0, dataLen = 0;
	
	res = fread(mcharLength, 4, 1, mFile);
	if(res>0)
	{
		dataLen = charsToInt1(mcharLength,0);
		res 	= fread(mData, dataLen, 1, mFile);
		
		VIDEOLOGD("startCodec------------3 res:%d dataLen:%d", res, dataLen);
		
		CodecBaseLib::getInstance()->AddBuffer(mData, dataLen);
		//mCodec->addBuffer(mData, dataLen);
		usleep(50*1000);
	}

	return true;
}



