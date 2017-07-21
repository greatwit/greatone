
#include "TestFileCodec.h"
#include "RtpReceive.h"


#include "ComDefine.h"
#define TAG "TestFileCodec"


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)


char *gFilePath1 = "/sdcard/camera.h264";

int charsToInt1(char* src, int offset) 
{  
	int value;
	value = (int) ((src[offset]	  & 0xFF)
				| ((src[offset+1] & 0xFF)<<8)
				| ((src[offset+2] & 0xFF)<<16)
				| ((src[offset+3] & 0xFF)<<24));
	return value;
} 


TestFileCodec::TestFileCodec()
			:mbRunning(false)
			,mFile(NULL)
{
	ALOGV("SenderServer::SenderServer() construct.");
}

TestFileCodec::~TestFileCodec()
{
	ALOGV("SenderServer, Destructor");
}

bool TestFileCodec::CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	mFile = fopen(gFilePath1, "rb");
	mcharLength[4] = {0};
	mData[1000000] = {0};
	

	return true;
}

bool TestFileCodec::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	mFile = fopen(gFilePath1, "rb");
	mcharLength[4] = {0};
	mData[1000000] = {0};
	
	mCodec = new CodecBase("video/avc", true, false);
	mCodec->CreateCodec(format, surface, crypto, flags);
	
	return true;
}

bool TestFileCodec::DeInit()
{	
	if(mbRunning)
		StopVideo();
	
	fclose(mFile);
	

	
	return true;
}


bool TestFileCodec::StartVideo(int deivceid)
{
	if(mbRunning)
		return false;

	mCodec->startCodec();

	
	run("SenderServer", PRIORITY_URGENT_DISPLAY);
	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);

	mbRunning = true;

	return true;
}

bool TestFileCodec::StopVideo()
{
	if(false==mbRunning)
		return false;

	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	mbRunning = false;
	requestExit();

	mCodec->stopCodec();
	
	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void TestFileCodec::onCodecBuffer(struct CodecBuffer& buff)
{
}


bool TestFileCodec::threadLoop()
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
		
		//decorder(mData, dataLen);
		mCodec->addBuffer(mData, dataLen);
		usleep(50*1000);
	}

	return true;
}



