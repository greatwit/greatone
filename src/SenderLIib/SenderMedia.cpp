


#include "android_runtime/AndroidRuntime.h"
#include "jni.h"
#include "RtpSender.h"

#define TAG "SenderMedia"



using namespace android;

#ifdef __cplusplus 
extern "C" 
{
#endif


static RtpSender     *mpSender	  = NULL;


/////////////////////////////////////////////////////////////////////////////////////sender///////////////////////////////////////////////////////////////////////////


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

#ifdef __cplusplus
}
#endif




static JNINativeMethod gMethods[] = 
{
	{ "SenderCreate", "(I)Z", (void *)SenderCreate },
	{ "SenderConnect", "(Ljava/lang/String;I)Z", (void *)SenderConnect },
	{ "SenderByteArray", "([BI)Z", (void *)SenderByteArray },
	{ "SenderDestroy", "()Z", (void *)SenderDestroy },
	
};

int jniRegisterNativeMethods2(JNIEnv* env,
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
	return jniRegisterNativeMethods2(env, "com/great/happyness/Codec/SenderMedia", gMethods, sizeof(gMethods) / sizeof(gMethods[0])); 
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
	
	return result;
}

