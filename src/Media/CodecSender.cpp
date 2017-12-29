
#include "CodecSender.h"



#include "ComDefine.h"
#define TAG "CodecSender"




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

bool CodecSender::CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort)
{

	return true;
}

bool CodecSender::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort, int cameraId)
{

	mpSender = new RtpSender();
	if(!mpSender->initSession(sendPort))
	{
		ALOGE("function %s,line:%d mpSender->initSession() failed.", __FUNCTION__, __LINE__);
		return false;
	}

	mCamera = Camera::connect(cameraId);
	// make sure camera hardware is alive
    if (mCamera->getStatus() != NO_ERROR) {
        ALOGE("Camera initialization failed");
    }
	mCamera->setListener(this);
	mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_CAMERA);
	if (mCamera->setPreviewDisplay(surface) != NO_ERROR){
		ALOGE("Camera setPreviewDisplay failed");
    }

	//mCodec = new CodecBase("video/avc", true, true);
	//mCodec->CreateCodec(format, surface, crypto, flags);
	//mCodec->registerBufferCall(this);
	
	return true;
}

bool CodecSender::DeInit()
{	
	StopVideo();
	//mCodec = NULL;
	
	mpSender->deinitSession();
	delete mpSender;
	mpSender = NULL;

	return true;
}

void CodecSender::notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
	
}

void CodecSender::postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
	if (dataPtr != NULL) {
		ssize_t offset;
		size_t size;
		sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
		//ALOGE("postData: off=%ld, size=%d", offset, size);
	}
}

void CodecSender::postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr)
{
	if (dataPtr != NULL) {
		ssize_t offset;
		size_t size;
		sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
		ALOGE("postDataTimestamp: off=%ld, size=%d", offset, size);
	}
}

void CodecSender::SetCameraParameter(String8 params8)
{
	if (mCamera->setParameters(params8) != NO_ERROR) {
		ALOGE("Camera setParameters failed");
    }
}

String8 CodecSender::GetCameraParameter()
{
	return mCamera->getParameters();
}

bool CodecSender::StartVideo()
{

	//mCodec->startCodec();
	
	if (mCamera->startPreview() != NO_ERROR) {
		ALOGE("Camera startPreview failed");
    }

	VIDEOLOGD("function %s,line:%d",__FUNCTION__,__LINE__);


	return true; 
}

bool CodecSender::StopVideo()
{
	ALOGW("function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	//mCodec->stopCodec();
	mCamera->stopPreview();

	if (mCamera != NULL) {
		mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_NOOP);
		mCamera->disconnect();
    }
	
	VIDEOLOGD("function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

bool CodecSender::ConnectDest(std::string ip, short port)
{
	bool bRes = false;

	bRes = mpSender->connect(ip, port);
	if (bRes == false)
	{
		ALOGE("function %s,line:%d connect device failed... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}
	else
	{
		ALOGD("function %s,line:%d connect device successfuled... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}

	return bRes;
}

void CodecSender::onCodecBuffer(struct CodecBuffer& buff)
{
	ALOGW("onCodecBuffer--size:%d flags:%d", buff.size, buff.flags);
	mpSender->sendBuffer(buff.buf, buff.size, MIME_H264, MIME_H264_LEN, 0);
}

void CodecSender::AddDecodecSource(char *data, int len)
{
	//mCodec->addBuffer(data, len);
}



