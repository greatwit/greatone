
#include "TestEncoder.h"
#include "RtpReceive.h"


#include "ComDefine.h"
#define TAG "TestEncoder"


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)




TestEncoder::TestEncoder()
			:mbRunning(false)
{
	ALOGV("SenderServer::SenderServer() construct.");
}

TestEncoder::~TestEncoder()
{
	ALOGV("SenderServer, Destructor");
}

bool TestEncoder::CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	mcharLength[4] = {0};
	mData[1000000] = {0};

	mThread = new EncodecThread(this);
	

	return true;
}

bool TestEncoder::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	mcharLength[4] = {0};
	mData[1000000] = {0};

	mThread = new EncodecThread(this);
	
	mCodec = new CodecBase("video/avc", true, true);
	mCodec->CreateCodec(format, surface, crypto, flags);
	
	return true;
}

bool TestEncoder::DeInit()
{	
	if(mbRunning)
		StopVideo();
	
	fclose(mFile);
	

	
	return true;
}


bool TestEncoder::StartVideo(int deivceid)
{
	if(mbRunning)
		return false;

	mCodec->startCodec();
	
	mThread->startThread();
	
	run("SenderServer", PRIORITY_URGENT_DISPLAY);
	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);

	mpCameraSource = new CameraHardware(); 
	mpCameraSource->SetPixelFormat(V4L2_PIX_FMT_NV12); //for encoder
	mpCameraSource->InitCamera(1280, 720, deivceid);
	mpCameraSource->RegisterCallback(this);
	mpCameraSource->SetFrameRate(VIDEO_FRAMERATE);
	mpCameraSource->StartCamera();

	mbRunning = true;

	return true;
}

bool TestEncoder::StopVideo()
{
	if(false==mbRunning)
		return false;

	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

		mpCameraSource->StopCamera();
		ALOGW("TAG 1,function %s,line:%d StopVideo 00",__FUNCTION__,__LINE__);
		mpCameraSource->DeInitCamera();
		ALOGW("TAG 1,function %s,line:%d StopVideo 000",__FUNCTION__,__LINE__);

	mbRunning = false;
	requestExit();

	mCodec->stopCodec();
	mThread->stopThread();
	
	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void TestEncoder::GetDecodecSource()
{
	usleep(50*1000);
	
	mCodec->getCodecBuffer();
	ALOGTEST("GetDecodecSource------------8");
	
}

void TestEncoder::decorder(char*data, int dataLen)
{
	ALOGTEST("startCodec------------0");
}

void TestEncoder::VideoSource(V4L2BUF_t *pBuf)
{
	ALOGTEST("VideoSource------------len:%d", pBuf->length);
}

bool TestEncoder::threadLoop()
{
	if(!mbRunning)
		return true;

	int res = 0, dataLen = 0;
	

	return true;
}



