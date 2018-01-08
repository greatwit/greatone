

#include <utils/Log.h>


#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "jni.h"
#include "JNIHelp.h"

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

#include "media_Utils.h"

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

static jboolean StartVideoSend(JNIEnv *env, jobject thiz, 						
						jobjectArray keys, jobjectArray values,
						jobject jsurface,
						jobject jcrypto,
						jint flags,
						jobject bufferInfo
						)
{
	ALOGE("Enter:StartVideoSend----------->1");

	return true;
}

static jboolean StopVideoSend(JNIEnv *env, jobject)
{
	int res = CodecBaseLib::getInstance()->testAdd(1,1);
	ALOGE("Enter:StartVideoSend----------->res:%d", res);
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
	ALOGE("Enter:StartFileDecoder----------->1");
	
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
		ALOGE("Enter:StartFileDecoder----------->2");
		mpFileDecoder->CreateCodec(format, surface, crypto, flags, (char *)file);
		ALOGE("Enter:StartFileDecoder----------->3");
		env->ReleaseStringUTFChars(filepath, file);

		ALOGE("Enter:StartFileDecoder----------->4");
		mpFileDecoder->StartVideo(0);
	}
	
	ALOGE("Enter:StartFileDecoder----------->5");
	
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
	
	sp<AMessage> format;
	status_t err = CodecBaseLib::getInstance()->ConvertKeyValueToMessage(env, keys, values, &format);//ConvertKeyValueArraysToMessage(env, keys, values, &format);

	

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	const char *file = env->GetStringUTFChars(filepath, NULL);
	
	mpFileEncoder = new FileEnCodec();
	ALOGE("Enter:StartFileEncoder----------->2");

	if(jsurface!=NULL)
	{
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		mpFileEncoder->CreateCodec(format, surface, crypto, flags, (char*)file);
		ALOGE("Enter:StartFileEncoder----------->31");
	}else
	{
		mpFileEncoder->CreateCodec(format, NULL, crypto, flags, (char*)file);
		ALOGE("Enter:StartFileEncoder----------->32");
	}

	env->ReleaseStringUTFChars(filepath, file);

	ALOGE("Enter:StartFileEncoder----------->4");
	mpFileEncoder->StartVideo(0);
	ALOGE("Enter:StartFileEncoder----------->5");
	
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
	ALOGE("Enter:StartCodecSender----------->1");
	
	//读取sdk版本
	char szSdkVer[32]={0};
	__system_property_get("ro.build.version.sdk", szSdkVer);
	ALOGE("sdk:%d",atoi(szSdkVer));
	CodecBaseLib::getInstance()->LoadBaseLib(atoi(szSdkVer));
	
	sp<AMessage> format;
	status_t err = CodecBaseLib::getInstance()->ConvertKeyValueToMessage(env, keys, values, &format);//ConvertKeyValueArraysToMessage(env, keys, values, &format);

	

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpCodecSend = new CodecSender();
	ALOGE("Enter:StartCodecSender----------->2");

	if(jsurface!=NULL)
	{
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		mpCodecSend->CreateCodec(thiz, format, surface, crypto, flags, localport, 0);
		ALOGE("Enter:StartCodecSender----------->31");
	}else
	{
		mpCodecSend->CreateCodec(thiz, format, NULL, crypto, flags, localport, 0);
		ALOGE("Enter:StartCodecSender----------->32");
	}

	const char *pAddr = env->GetStringUTFChars(destip, NULL);
	mpCodecSend->ConnectDest(pAddr, destport);
	env->ReleaseStringUTFChars(destip, pAddr);
	
	
    return true;
}

static jboolean StartCameraVideo(JNIEnv *env, jobject thiz, jobject jsurface)
{
	sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
	mpCodecSend->StartVideo(surface);
	ALOGE("StartCameraVideo");
	return true;
}

static jboolean StopCameraVideo(JNIEnv *env, jobject)
{
	mpCodecSend->StopVideo();
	ALOGE("StopCameraVideo");
	return true;
}

static jstring GetCameraParameter(JNIEnv *env, jobject)
{
    ALOGE("GetCameraParameter");
	
    return mpCodecSend->GetCameraParameter();
}

static jboolean SetCameraParameter(JNIEnv *env, jobject, jstring params)
{
    mpCodecSend->SetCameraParameter(params);
	return true;
}

static jboolean CodecSenderData(JNIEnv *env, jobject, jbyteArray byteData, jint len)
{
	jbyte* cameraFrame = env->GetByteArrayElements(byteData, NULL);
	mpCodecSend->AddDecodecSource(reinterpret_cast<char*>(cameraFrame), len);
	env->ReleaseByteArrayElements(byteData, cameraFrame, JNI_ABORT);
	
	return true;
}

static jboolean StopCodecSender(JNIEnv *env, jobject)
{
	mpCodecSend->StopVideo();
	mpCodecSend->DeInit();
	delete mpCodecSend;
	mpCodecSend = NULL;
	
	return true;
}



//////////////////////////////////////////////////////////////////////////recerver//////////////////////////////////////////////////////////////

static jboolean StartCodecRecver(JNIEnv *env, jobject, 
					jobjectArray keys, jobjectArray values,
					jobject jsurface, jobject jcrypto, jint flags,
					jshort recvport)
{
	ALOGE("Enter:StartCodecRecver----------->1");
	
	sp<AMessage> format;
	status_t err = CodecBaseLib::getInstance()->ConvertKeyValueToMessage(env, keys, values, &format);//ConvertKeyValueArraysToMessage(env, keys, values, &format);

	sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpCodecRecv = new CodecReceiver();
	ALOGE("Enter:StartCodecRecver----------->2");
	mpCodecRecv->CreateCodec(format, surface, crypto, flags, recvport);
	ALOGE("Enter:StartCodecRecver----------->3");


	ALOGE("Enter:StartCodecRecver----------->4");
	mpCodecRecv->StartVideo(0);
	ALOGE("Enter:StartCodecRecver----------->5");

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

	//
	{ "StartCodecSender",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;Ljava/lang/String;SSI)Z",
      	(void *)StartCodecSender },
	{ "StopCodecSender", "()Z", (void *)StopCodecSender },
	{ "StartCameraVideo", "(Landroid/view/Surface;)Z", (void *)StartCameraVideo },
	{ "GetCameraParameter", "()Ljava/lang/String;", (void *)GetCameraParameter },
	{ "SetCameraParameter", "(Ljava/lang/String;)Z", (void *)SetCameraParameter },
	{ "CodecSenderData", "([BI)Z", (void *)CodecSenderData },
	
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

	ALOGTEST("Registering %s natives\n", className);
	clazz = env->FindClass(className);

	ALOGTEST("Registering %s natives 1\n", className);
	if (clazz == NULL) 
	{
		ALOGE("Native registration unable to find class '%s'\n", className);
		return -1;
	}

	ALOGTEST("Registering %s natives 2\n", className);
	if (env->RegisterNatives(clazz, methods, numMethods) < 0) 
	{
		ALOGE("RegisterNatives failed for '%s'\n", className);
		return -1;
	}
	ALOGTEST("Registering %s natives 3\n", className);

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

	ALOGTEST("loading . . .1");
	
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) 
	{
		ALOGE("GetEnv failed!");
		return result;           
	}
	
	if( registerNatives(env) != JNI_OK) 
	{
		ALOGE("can't load ffmpeg");
	}
	
	//if( registerCamera(env) != JNI_OK) 
	{
		ALOGE("registerCamera failed");
	}
	
	
	ALOGTEST( "loading . . .2");
	
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


