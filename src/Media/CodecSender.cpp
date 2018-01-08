
#include "CodecSender.h"

#include <sys/system_properties.h>

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

bool CodecSender::CreateCodec(jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort, int cameraId)
{
	mpSender = new RtpSender();
	if(!mpSender->initSession(sendPort))
	{
		ALOGE("function %s,line:%d mpSender->initSession() failed.", __FUNCTION__, __LINE__);
		return false;
	}


   //读取sdk版本
   char szSdkVer[32]={0};
   __system_property_get("ro.build.version.sdk", szSdkVer);
   ALOGE("sdk:%d",atoi(szSdkVer));
	
	CodecBaseLib::getInstance()->CodecCreate(format, NULL, crypto, flags, true);
	CodecBaseLib::getInstance()->RegisterBufferCall(this);
	
	JNIEnv *env = AndroidRuntime::getJNIEnv();
	jstring clientPackageName = env->NewStringUTF("com.greatmedia");
	CameraLib::getInstance()->LoadCameraLib(atoi(szSdkVer));
	CameraLib::getInstance()->CameraSetup(this, cameraId, clientPackageName);
	//mCamera = Camera::connect(cameraId);
	// make sure camera hardware is alive
    //if (mCamera->getStatus() != NO_ERROR) {
    //    ALOGE("Camera initialization failed");
    //}
	//mContext->incStrong(thiz);
	//mCamera->setListener(this);
	//mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER);
	//if (mCamera->setPreviewDisplay(surface) != NO_ERROR){
	//	ALOGE("Camera setPreviewDisplay failed");
    //}

	//mCodec = new CodecBase("video/avc", true, true);
	//mCodec->CreateCodec(format, surface, crypto, flags);
	//mCodec->registerBufferCall(this);
	
	return true;
}

bool CodecSender::DeInit()
{	
	//StopVideo();
	//mCodec = NULL;
	
	mpSender->deinitSession();
	delete mpSender;
	mpSender = NULL;

	return true;
}


void CodecSender::SetCameraParameter(jstring params)
{
	CameraLib::getInstance()->SetCameraParameter(params);
}

jstring CodecSender::GetCameraParameter()
{
    return CameraLib::getInstance()->GetCameraParameter();
}

bool CodecSender::StartVideo(const sp<Surface> &cameraSurf)
{

	//mCodec->startCodec();
	CodecBaseLib::getInstance()->StartCodec();
	
	CameraLib::getInstance()->StartPreview(cameraSurf);

	VIDEOLOGD("function %s,line:%d",__FUNCTION__,__LINE__);


	return true; 
}

bool CodecSender::StopVideo()
{
	ALOGW("function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	//mCodec->stopCodec();
	CodecBaseLib::getInstance()->StopCodec();
	
	CameraLib::getInstance()->StopPreview();
	CameraLib::getInstance()->CameraRelease();

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

//camera frame callback
void CodecSender::VideoSource(V4L2BUF_t *pBuf)
{
	ALOGW("function %s,line:%d len:%d", __FUNCTION__, __LINE__, pBuf->length);
	CodecBaseLib::getInstance()->AddBuffer((char*)pBuf->addrVirY, pBuf->length);
}

//codec frame callback
void CodecSender::onCodecBuffer(struct CodecBuffer& buff)
{
	ALOGW("onCodecBuffer--size:%d flags:%d", buff.size, buff.flags);
	//mpSender->sendBuffer(buff.buf, buff.size, MIME_H264, MIME_H264_LEN, 0);
}

//upload camera framebuffer
void CodecSender::AddDecodecSource(char *data, int len)
{
	//mCodec->addBuffer(data, len);
}



