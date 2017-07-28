
#include "TestEncoder.h"
#include "RtpReceive.h"


#include "ComDefine.h"
#define TAG "TestEncoder"


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)	__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)




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

	

	return true;
}

bool TestEncoder::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	mcharLength[4] = {0};
	mData[1000000] = {0};

	
	mCodec = new CodecBase("video/avc", true, true);
	mCodec->CreateCodec(format, surface, crypto, flags);
	mCodec->registerBufferCall(this);
	
	return true;
}

bool TestEncoder::DeInit()
{	
	if(mbRunning)
		StopVideo();
	
	//fclose(mFile);
	

	return true;
}


bool TestEncoder::StartVideo(int deivceid)
{

	mCodec->startCodec();

	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);


	return true;
}

bool TestEncoder::StopVideo()
{

	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	mCodec->stopCodec();
	
	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void TestEncoder::onCodecBuffer(struct CodecBuffer& buff)
{
	ALOGTEST("onCodecBuffer--size:%d flags:%d", buff.size, buff.flags);
}

void TestEncoder::AddDecodecSource(char *data, int len)
{
	mCodec->addBuffer(data, len);
}



