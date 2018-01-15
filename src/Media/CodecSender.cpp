
#include "CodecSender.h"

#include <sys/system_properties.h>

#include "ComDefine.h"
#define TAG "CodecSender"




CodecSender::CodecSender()
	    :mbRunning(false)
	    ,mpSender(NULL)
{ 
	GLOGV("function %s,line:%d construct.",__FUNCTION__,__LINE__);
}

CodecSender::~CodecSender()
{
	GLOGV("function %s,line:%d Destructor.",__FUNCTION__,__LINE__);
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
		GLOGE("function %s,line:%d mpSender->initSession() failed.", __FUNCTION__, __LINE__);
		return false;
	}

	bool bResult = false;
    //读取sdk版本
    char szSdkVer[32]={0};
    __system_property_get("ro.build.version.sdk", szSdkVer);
    GLOGW("sdk:%d",atoi(szSdkVer));
	
	JNIEnv *env = AndroidRuntime::getJNIEnv();
	jstring clientPackageName = env->NewStringUTF("com.greatmedia");
	bResult = CameraLib::getInstance()->LoadCameraLib(atoi(szSdkVer));
	if(bResult)
	{
		int camSet = CameraLib::getInstance()->CameraSetup(this, cameraId, clientPackageName);
		if(camSet>=0)
		{
			bResult = CodecBaseLib::getInstance()->CodecCreate(format, NULL, crypto, flags, true);
			if(bResult)
				CodecBaseLib::getInstance()->RegisterBufferCall(this);
			else
				GLOGE("function %s,line:%d CodecCreate failed.", __FUNCTION__, __LINE__);
		}
		else
		{
			GLOGE("function %s,line:%d CameraSetup failed.", __FUNCTION__, __LINE__);
			return false;
		}
	}
	else
		GLOGE("function %s,line:%d LoadCameraLib failed.", __FUNCTION__, __LINE__);

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
	
	return bResult;
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

void CodecSender::SetDisplayOrientation(int value)
{
	CameraLib::getInstance()->SetDisplayOrientation(value);
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

	GLOGD("function %s,line:%d",__FUNCTION__,__LINE__);

	return true; 
}

bool CodecSender::StopVideo()
{
	GLOGW("function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	//mCodec->stopCodec();
	CodecBaseLib::getInstance()->StopCodec();
	
	CameraLib::getInstance()->StopPreview();
	CameraLib::getInstance()->CameraRelease();

	GLOGD("function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

bool CodecSender::ConnectDest(std::string ip, short port)
{
	bool bRes = false;

	bRes = mpSender->connect(ip, port);
	if (bRes == false)
	{
		GLOGE("function %s,line:%d connect device failed... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}
	else
	{
		GLOGD("function %s,line:%d connect device successfuled... ip:%s port:%d", __FUNCTION__, __LINE__, ip.c_str(), port);
	}

	return bRes;
}

//camera frame callback
void CodecSender::VideoSource(V4L2BUF_t *pBuf)
{
	GLOGW("function %s,line:%d len:%d", __FUNCTION__, __LINE__, pBuf->length);
	char* data = (char*)pBuf->addrVirY;
	
	int ylen  = pBuf->length*2/3;
	int uvlen = ylen/2;
	char tmp  = '/0';
	for(int i=0;i<uvlen;) //NV21 to NV12
	{
		tmp 			= data[ylen+i];
		data[ylen+i] 	= data[ylen+i+1];
		data[ylen+i+1] 	= tmp;
		i+=2;
	}
	
	CodecBaseLib::getInstance()->AddBuffer(data, pBuf->length);
}

//codec frame callback
void CodecSender::onCodecBuffer(struct CodecBuffer& buff)
{
	GLOGW("function %s,line:%d onCodecBuffer size:%d flags:%d", __FUNCTION__, __LINE__, buff.size, buff.flags);
	mpSender->sendBuffer(buff.buf, buff.size, MIME_H264, MIME_H264_LEN, 0);
}

//upload camera framebuffer
void CodecSender::AddDecodecSource(char *data, int len)
{
	//mCodec->addBuffer(data, len);
}



