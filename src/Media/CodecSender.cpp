
#include "CodecSender.h"

#include "ComDefine.h"
#define TAG "CodecSender"


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)	__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)




CodecSender::CodecSender()
	    :mbRunning(false)
	    ,mpSender(NULL)
{ 
	ALOGV("SenderServer::SenderServer() construct.");
}

CodecSender::~CodecSender()
{
	ALOGV("SenderServer, Destructor");
}

bool CodecSender::CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{

	return true;
}

bool CodecSender::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort)
{

	mpSender = new RtpSender();
	if(!mpSender->initSession(sendPort))
	{
		ALOGE("TAG 2,function %s,line:%d mpSender->initSession() failed.", __FUNCTION__, __LINE__);
		return false;
	}

	mCodec = new CodecBase("video/avc", true, true);
	mCodec->CreateCodec(format, surface, crypto, flags);
	mCodec->registerBufferCall(this);
	
	return true;
}

bool CodecSender::DeInit()
{	
	StopVideo();
	mCodec = NULL;
	
	mpSender->deinitSession();
	delete mpSender;
	mpSender = NULL;

	return true;
}


bool CodecSender::StartVideo(int deivceid)
{

	mCodec->startCodec();

	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);


	return true;
}

bool CodecSender::StopVideo()
{
	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	mCodec->stopCodec();
	
	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

bool CodecSender::ConnectDest(std::string ip, short port)
{
	bool bRes = false;

	bRes = mpSender->connect(ip, port);
	if (bRes == false)
	{
		ALOGE("TAG 2,function %s,line:%d connect device failed... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}
	else
	{
		ALOGD("TAG 2,function %s,line:%d connect device successfuled... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}

	return bRes;
}

void CodecSender::onCodecBuffer(struct CodecBuffer& buff)
{
	ALOGTEST("onCodecBuffer--size:%d flags:%d", buff.size, buff.flags);
	mpSender->sendBuffer(buff.buf, buff.size, MIME_H264, MIME_H264_LEN, 0);
}

void CodecSender::AddDecodecSource(char *data, int len)
{
	mCodec->addBuffer(data, len);
}



