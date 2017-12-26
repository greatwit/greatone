
#include "CodecReceiver.h"
#include "RtpReceive.h"


#include "ComDefine.h"
#define TAG "CodecReceiver"


CodecReceiver::CodecReceiver()
	      :mbRunning(false)
	      ,mpReceive(NULL)
{
	ALOGV("SenderServer::SenderServer() construct.");
}

CodecReceiver::~CodecReceiver()
{
	ALOGV("SenderServer, Destructor");
}

bool CodecReceiver::CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags)
{
	return true;
}

bool CodecReceiver::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short recvPort)
{
	mpReceive = new RtpReceive();
	if(mpReceive->initSession(recvPort))
	{
		mpReceive->registerCallback(this);
	}
	else
	{
		ALOGE("TAG 2,function %s,line:%d  RtpReceive::Singleton->initSession failed",__FUNCTION__,__LINE__);
	}

	//mCodec = new CodecBase("video/avc", true, false);
	//mCodec->CreateCodec(format, surface, crypto, flags);
	CodecBaseLib::getInstance()->CodecCreate(format, surface, crypto, flags, false);
	
	return true;
}

bool CodecReceiver::DeInit()
{	
	StopVideo();
	mpReceive->deinitSession();
	SAFE_DELETE(mpReceive);
	//mCodec = NULL;
	return true;
}

bool CodecReceiver::StartVideo(int deivceid)
{
	//mCodec->startCodec();
	CodecBaseLib::getInstance()->StartCodec();
	
	mpReceive->startThread();
	
	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);

	return true;
}

bool CodecReceiver::StopVideo()
{
	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	mpReceive->stopThread();
	
	//mCodec->stopCodec();
	CodecBaseLib::getInstance()->StopCodec();
	
	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void CodecReceiver::ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen)
{
	if(0 == strcmp(mimeType, MIME_H264))
	{
		CodecBaseLib::getInstance()->AddBuffer((char*)buffer, dataLen);
		//mCodec->addBuffer((char*)buffer, dataLen);
	}

	return;
}




