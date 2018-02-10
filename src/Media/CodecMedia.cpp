


#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "jni.h"

#include "cutils/properties.h"
#include <gui/Surface.h>


#include <media/ICrypto.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>

#include <sys/system_properties.h>

#include "FileDeCodec.h"
#include "FileEnCodec.h"
#include "CodecReceiver.h"
#include "CodecSender.h"


#include "CodecMedia.h"
#define TAG "CodecMedia"



using namespace android;

#ifdef __cplusplus
extern "C" 
{
#endif


static sp<FileDeCodec>    mpFileDecoder 	= NULL;
static FileEnCodec        *mpFileEncoder 	= NULL;

 
static CodecReceiver *mpCodecRecv = NULL;
static CodecSender   *mpCodecSend = NULL;
static RtpSender     *mpSender	  = NULL;


static jboolean StartVideoSend(JNIEnv *env, jobject thiz, 						
						jobjectArray keys, jobjectArray values,
						jobject jsurface,
						jobject jcrypto,
						jint flags,
						jobject bufferInfo
						)
{
	GLOGW("Enter:StartVideoSend----------->1");

	return true;
}

static jboolean StopVideoSend(JNIEnv *env, jobject)
{
	int res = CodecBaseLib::getInstance()->testAdd(1,1);
	GLOGW("Enter:StartVideoSend----------->res:%d", res);
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////file codec//////////////////////////////////////////////////////////////////////////////////////////////



static jboolean StartFileDecoder(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags,
											jstring filepath)
{
	GLOGW("Enter:StartFileDecoder----------->1");
	
	sp<AMessage> format;
	status_t err = CodecBaseLib::getInstance()->ConvertKeyValueToMessage(env, keys, values, &format);//ConvertKeyValueArraysToMessage(env, keys, values, &format);

	sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	
	if(mpFileDecoder == NULL)
	{
		const char *file = env->GetStringUTFChars(filepath, NULL);
	
		mpFileDecoder = new FileDeCodec();
		ALOGW("Enter:StartFileDecoder----------->2");
		mpFileDecoder->CreateCodec(format, surface, crypto, flags, (char *)file);
		ALOGW("Enter:StartFileDecoder----------->3");
		env->ReleaseStringUTFChars(filepath, file);

		ALOGW("Enter:StartFileDecoder----------->4");
		mpFileDecoder->StartVideo(0);
	}
	
	GLOGW("Enter:StartFileDecoder----------->5");
	
	return true;
}

static jboolean StopFileDecoder(JNIEnv *env, jobject)
{
	if(mpFileDecoder!=NULL)
	{
		mpFileDecoder->StopVideo();
		mpFileDecoder->DeInit();
		mpFileDecoder = NULL;
		return true;
	}
	return false;
}


static jboolean StartFileEncoder(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags,
											jstring filepath)
{
	ALOGE("Enter:StartFileEncoder----------->1");
	
	//读取sdk版本
	char szSdkVer[32]={0};
	__system_property_get("ro.build.version.sdk", szSdkVer);
	GLOGI("sdk:%d",atoi(szSdkVer));
	CodecBaseLib::getInstance()->LoadBaseLib(atoi(szSdkVer));
	
	sp<AMessage> format;
	status_t err = CodecBaseLib::getInstance()->ConvertKeyValueToMessage(env, keys, values, &format);//ConvertKeyValueArraysToMessage(env, keys, values, &format);

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	const char *file = env->GetStringUTFChars(filepath, NULL);
	
	mpFileEncoder = new FileEnCodec();
	GLOGW("Enter:StartFileEncoder----------->2");

	if(jsurface!=NULL)
	{
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		mpFileEncoder->CreateCodec(format, surface, crypto, flags, (char*)file);
		GLOGE("Enter:StartFileEncoder----------->31");
	}else
	{
		mpFileEncoder->CreateCodec(format, NULL, crypto, flags, (char*)file);
		GLOGE("Enter:StartFileEncoder----------->32");
	}

	env->ReleaseStringUTFChars(filepath, file);

	GLOGW("Enter:StartFileEncoder----------->4");
	mpFileEncoder->StartVideo(0);
	GLOGW("Enter:StartFileEncoder----------->5");
	
	return true;
}

static jboolean AddEncoderData(JNIEnv *env, jobject, jbyteArray byteData, jint len)
{
	jbyte* cameraFrame = env->GetByteArrayElements(byteData, NULL);
	mpFileEncoder->AddDecodecSource(reinterpret_cast<char*>(cameraFrame), len);
	env->ReleaseByteArrayElements(byteData, cameraFrame, JNI_ABORT);
	return true;
}

static jboolean StopFileEncoder(JNIEnv *env, jobject)
{
	mpFileEncoder->StopVideo();
	return mpFileEncoder->DeInit();
}



/////////////////////////////////////////////////////////////////////////////////////sender///////////////////////////////////////////////////////////////////////////

static jboolean StartCodecSender(JNIEnv *env, jobject thiz, 
					jobjectArray keys, jobjectArray values,
					jobject jsurface, jobject jcrypto, jstring destip, 
					jshort destport, jshort localport, jint flags)
{
	GLOGW("Enter:StartCodecSender----------->1");
	bool bResult = false;
	if(mpCodecSend == NULL)
	{
		//读取sdk版本
		char szSdkVer[32]={0};
		__system_property_get("ro.build.version.sdk", szSdkVer);
		GLOGI("sdk:%d",atoi(szSdkVer));
		CodecBaseLib::getInstance()->LoadBaseLib(atoi(szSdkVer));
		
		sp<AMessage> format;
		status_t err = CodecBaseLib::getInstance()->ConvertKeyValueToMessage(env, keys, values, &format);//ConvertKeyValueArraysToMessage(env, keys, values, &format);

		

		sp<ICrypto> crypto;
		if (jcrypto != NULL) {
		//crypto = JCrypto::GetCrypto(env, jcrypto);
		}
		
		mpCodecSend = new CodecSender();
		GLOGD("Enter:StartCodecSender----------->2");

		if(jsurface!=NULL)
		{
			sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
			bResult = mpCodecSend->CreateCodec(thiz, format, surface, crypto, flags, localport, 0);
			GLOGD("Enter:StartCodecSender----------->31");
		}else
		{
			bResult = mpCodecSend->CreateCodec(thiz, format, NULL, crypto, flags, localport, 0);
			GLOGD("Enter:StartCodecSender----------->32");
		}

		if(bResult)
		{
			const char *pAddr = env->GetStringUTFChars(destip, NULL);
			mpCodecSend->ConnectDest(pAddr, destport);
			env->ReleaseStringUTFChars(destip, pAddr);
		}
		else
		{
			GLOGE("function %s,line:%d StartCodecSender failed.", __FUNCTION__, __LINE__);
			return false;
		}
	}

    return true;
}

static jboolean StartCameraVideo(JNIEnv *env, jobject thiz, jobject jsurface)
{
	if(mpCodecSend)
	{
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		mpCodecSend->StartVideo(surface);
		GLOGE("StartCameraVideo");
		return true;
	}
	return false;
}

static jboolean StopCameraVideo(JNIEnv *env, jobject)
{
	if(mpCodecSend)
		mpCodecSend->StopVideo();
	GLOGW("StopCameraVideo");
	return true;
}

static jstring GetCameraParameter(JNIEnv *env, jobject)
{
    GLOGW("GetCameraParameter");
	if(mpCodecSend)
		return mpCodecSend->GetCameraParameter();
	else
		return NULL;
}

static jboolean SetCameraParameter(JNIEnv *env, jobject, jstring params)
{
	if(mpCodecSend)
		mpCodecSend->SetCameraParameter(params);
	return true;
}

static jboolean SetDisplayOrientation(JNIEnv *env, jobject, jint value)
{
	if(mpCodecSend)
		mpCodecSend->SetDisplayOrientation(value);
	return true;
}

static jboolean CodecSenderData(JNIEnv *env, jobject, jbyteArray byteData, jint len)
{
	if(mpCodecSend)
	{
		jbyte* cameraFrame = env->GetByteArrayElements(byteData, NULL);
		mpCodecSend->AddDecodecSource(reinterpret_cast<char*>(cameraFrame), len);
		env->ReleaseByteArrayElements(byteData, cameraFrame, JNI_ABORT);
	
		return true;
	}
	return false;
}

static jboolean StopCodecSender(JNIEnv *env, jobject)
{
	if(mpCodecSend)
	{
		mpCodecSend->StopVideo();
		mpCodecSend->DeInit();
		delete mpCodecSend;
		mpCodecSend = NULL;
		
		return true;
	}
	return false;
}



static jboolean SenderCreate(JNIEnv *env, jobject, int sendPort)
{
	if(mpSender==NULL)
		mpSender = new RtpSender();
	if(!mpSender->initSession(sendPort))
	{
		GLOGE("function %s,line:%d mpSender->initSession() failed.", __FUNCTION__, __LINE__);
		return false;
	}
	return true;
}

static jboolean SenderConnect(JNIEnv *env, jobject, std::string ip, int port)
{
	bool bRes = false;
	if(mpSender)
		bRes = mpSender->connect(ip, port);
	return bRes;
}

static jboolean SenderByteArray(JNIEnv *env, jobject, jbyteArray byteData, jint len)
{
	if(mpSender)
	{
		jbyte* cameraFrame = env->GetByteArrayElements(byteData, NULL);
		mpSender->sendBuffer(reinterpret_cast<char*>(cameraFrame), len, MIME_H264, MIME_H264_LEN, 0);
		env->ReleaseByteArrayElements(byteData, cameraFrame, JNI_ABORT);
	
		return true;
	}
	return false;
}

static jboolean SenderDestroy(JNIEnv *env, jobject)
{
	if(mpSender)
	{
		mpSender->deinitSession();
		delete mpSender;
		mpSender = NULL;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////recerver//////////////////////////////////////////////////////////////

static jboolean StartCodecRecver(JNIEnv *env, jobject, 
					jobjectArray keys, jobjectArray values,
					jobject jsurface, jobject jcrypto, jint flags,
					jshort recvport)
{
	GLOGW("Enter:StartCodecRecver----------->1");
	
	//读取sdk版本
	char szSdkVer[32]={0};
	__system_property_get("ro.build.version.sdk", szSdkVer);
	GLOGI("sdk:%d",atoi(szSdkVer));
	CodecBaseLib::getInstance()->LoadBaseLib(atoi(szSdkVer));
	
	sp<AMessage> format;
	status_t err = CodecBaseLib::getInstance()->ConvertKeyValueToMessage(env, keys, values, &format);//ConvertKeyValueArraysToMessage(env, keys, values, &format);
	GLOGD("Enter:StartCodecRecver----------->2");
	sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
	
	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpCodecRecv = new CodecReceiver();
	
	mpCodecRecv->CreateCodec(format, surface, crypto, flags, recvport);

	GLOGD("Enter:StartCodecRecver----------->4");
	mpCodecRecv->StartVideo(0);
	GLOGD("Enter:StartCodecRecver----------->5");

	return true;
}

static jboolean StopCodecRecver(JNIEnv *env, jobject)
{
	mpCodecRecv->StopVideo();
	mpCodecRecv->DeInit();
	delete mpCodecRecv;
	mpCodecRecv = NULL;
	return true;
}

static jboolean LoadBaseLib(JNIEnv *env, jobject obj, int version)
{
	return CodecBaseLib::getInstance()->LoadBaseLib(version);
}

#ifdef __cplusplus
}
#endif




static JNINativeMethod gMethods[] = 
{
	//{ "LoadBaseLib", "(I)Z", (void *)LoadBaseLib },
	
	{ "StopVideoSend", "()Z", (void *)StopVideoSend },
	
        { "StartFileDecoder",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;ILjava/lang/String;)Z",
      	(void *)StartFileDecoder },
	{ "StopFileDecoder", "()Z", (void *)StopFileDecoder },

	//
	{ "StartFileEncoder",
		"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
		"Landroid/media/MediaCrypto;ILjava/lang/String;)Z",
		(void *)StartFileEncoder },
	{ "StopFileEncoder", "()Z", (void *)StopFileEncoder },
	{ "AddEncoderData", "([BI)Z", (void *)AddEncoderData },

	//sender
	{ "StartCodecSender",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;Ljava/lang/String;SSI)Z",
      	(void *)StartCodecSender },
	{ "StopCodecSender", "()Z", (void *)StopCodecSender },
	{ "StartCameraVideo", "(Landroid/view/Surface;)Z", (void *)StartCameraVideo },
	{ "GetCameraParameter", "()Ljava/lang/String;", (void *)GetCameraParameter },
	{ "SetCameraParameter", "(Ljava/lang/String;)Z", (void *)SetCameraParameter },
	{ "SetDisplayOrientation", "(I)Z", (void *)SetDisplayOrientation },
	{ "CodecSenderData", "([BI)Z", (void *)CodecSenderData },

	{ "SenderCreate", "(I)Z", (void *)SenderCreate },
	{ "SenderConnect", "(Ljava/lang/String;I)Z", (void *)SenderConnect },
	{ "SenderByteArray", "([BI)Z", (void *)SenderByteArray },
	{ "SenderDestroy", "()Z", (void *)SenderDestroy },
	
	//
	{ "StartCodecRecver",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;IS)Z",
      	(void *)StartCodecRecver },
	{ "StopCodecRecver", "()Z", (void *)StopCodecRecver },
	
};

int jniRegisterNativeMethods1(JNIEnv* env,
							 const char* className,
							 const JNINativeMethod* methods,
							 int numMethods)
{
	jclass clazz;

	GLOGW("Registering %s natives\n", className);
	clazz = env->FindClass(className);

	GLOGW("Registering %s natives 1\n", className);
	if (clazz == NULL) 
	{
		GLOGE("Native registration unable to find class '%s'\n", className);
		return -1;
	}

	GLOGW("Registering %s natives 2\n", className);
	if (env->RegisterNatives(clazz, methods, numMethods) < 0) 
	{
		GLOGE("RegisterNatives failed for '%s'\n", className);
		return -1;
	}
	GLOGW("Registering %s natives 3\n", className);

	return 0;
}

int registerNatives(JNIEnv *env) 
{
	return jniRegisterNativeMethods1(env, "com/great/happyness/Codec/CodecMedia", gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) 
{
	JNIEnv* env = NULL;
	jint result = JNI_ERR;

	GLOGW("loading . . .1");
	
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) 
	{
		GLOGE("GetEnv failed!");
		return result;           
	}
	
	if( registerNatives(env) != JNI_OK) 
	{
		GLOGE("can't load ffmpeg");
	}
	
	
	GLOGW( "loading . . .2");
	
	result = JNI_VERSION_1_4;
	char prop[PROPERTY_VALUE_MAX] = "000";
	if(property_get("prop_name", prop, NULL) != 0)
	{
	}
	
	return result;
}

//int register_Java_com_great_happyness_Codec_CodecMedia(JNIEnv *env) 
//{
//    return AndroidRuntime::registerNativeMethods(env, "Java/com/great/happyness/Codec/CodecMedia", gMethods, NELEM(gMethods));
//}


