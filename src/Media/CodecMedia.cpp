

#include <utils/Log.h>

#include "media_MediaCodec.h" 
#include "media_Utils.h"

#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "jni.h"
#include "JNIHelp.h"

#include "cutils/properties.h"
#include <gui/Surface.h>

#if HEIGHT_VERSION
#else
#include <gui/SurfaceTextureClient.h>
#endif


#include <media/ICrypto.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>


#include "TestFileCodec.h"
#include "FilesDecodec.h"
#include "VideoReceiver.h"


#include "CodecMedia.h"
#define TAG "CodecMedia"

#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)

#define PROPERTY_KEY_MAX    32
#define PROPERTY_VALUE_MAX  92

char *mFilePath = "/sdcard/camera.h264";


using namespace android;

#ifdef __cplusplus
extern "C" 
{
#endif



uint8_t*bufferPoint[2];
sp<JMediaCodec>  mCodec = NULL;


void JNI_API_NAME(native_configure)( JNIEnv *env, jobject thiz,
										jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags)
{
	mCodec = new JMediaCodec(env, thiz, "video/avc", true, false);
	status_t err = mCodec->initCheck();
	
    sp<AMessage> format;
    err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

    if (err != OK) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

	#if HEIGHT_VERSION
	    sp<IGraphicBufferProducer> bufferProducer;
	    if (jsurface != NULL) {
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		if (surface != NULL) {
		    bufferProducer = surface->getIGraphicBufferProducer();
		} else {
		    jniThrowException(
		            env,
		            "java/lang/IllegalArgumentException",
		            "The surface has been released");
		    return;
		}
	    }

	    sp<ICrypto> crypto;
	    if (jcrypto != NULL) {
		//crypto = JCrypto::GetCrypto(env, jcrypto);
	    }

	    err = mCodec->configure(format, bufferProducer, crypto, flags);
	#else
	    sp<ISurfaceTexture> surfaceTexture;
	    if (jsurface != NULL) 
		{
			sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
			if (surface != NULL) 
			{
				surfaceTexture = surface->getSurfaceTexture();
			} 
			else 
			{
				jniThrowException(
						env,
						"java/lang/IllegalArgumentException",
						"The surface has been released");
				return;
			}
	    }

	    sp<ICrypto> crypto;
	    if (jcrypto != NULL) {
		//crypto = JCrypto::GetCrypto(env, jcrypto);
	    }

	    err = mCodec->configure(format, surfaceTexture, crypto, flags);
	#endif

	    err = mCodec->start();

		jobjectArray inputBuffers;
		err = mCodec->getBuffers(env, true, &inputBuffers);

		for(int i =0;i<2;i++)
		{
			jobject inputObject = env->GetObjectArrayElement(inputBuffers, i);
			bufferPoint[i] 	= (uint8_t*)env->GetDirectBufferAddress( inputObject);
		}

}

int bytesToInt(char* src, int offset) 
{  
	int value;    
	value = (int) ((src[offset]	  & 0xFF)   
				| ((src[offset+1] & 0xFF)<<8)   
				| ((src[offset+2] & 0xFF)<<16)   
				| ((src[offset+3] & 0xFF)<<24));  
	return value;  
} 

void decorder(JNIEnv *env, jobject thiz, char*data, int dataLen)
{
	ALOGTEST("startCodec------------0");
	
	size_t inputBufferIndex = 0, outputBufferIndex = 0;
	
	status_t err;
	int mCount = 0;
	AString errorDetailMsg;
	
	
	err = mCodec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("startCodec------------4 err:%d", err);


	uint8_t* inputChar = bufferPoint[inputBufferIndex];
	memcpy(inputChar, data, dataLen);
	
	err = mCodec->queueInputBuffer(inputBufferIndex, 0, dataLen, mCount * 1000000 / 20, 0, &errorDetailMsg);
	mCount++;
	
	ALOGTEST("startCodec------------6 err:%d input index:%d inputChar addr:%d", err, inputBufferIndex, inputChar);
	
	
	err = mCodec->dequeueOutputBuffer(env, NULL, &outputBufferIndex, 0);
	ALOGTEST("startCodec------------7 err:%d outputBufferIndex:%d", err, outputBufferIndex);
	
	while (err ==0 && outputBufferIndex >= 0) 
	{
		#if HEIGHT_VERSION
		    mCodec->releaseOutputBuffer(outputBufferIndex, true, false, 0);
		#else
		    mCodec->releaseOutputBuffer(outputBufferIndex, true);
		#endif
		err = mCodec->dequeueOutputBuffer(env, NULL, &outputBufferIndex, 0);
	}

	if (outputBufferIndex < 0) 
	{
		ALOGTEST("startCodec------------err :%d ", outputBufferIndex);
	}
	
	ALOGTEST("startCodec------------8");
}

void JNI_API_NAME(startCodec)( JNIEnv *env, jobject thiz)
{
	char length[4] = {0};
	char data[1000000] = {0};

	FILE *file = fopen(mFilePath, "rb");
	int res = 0, dataLen = 0;
    
	do 
	{
		res = fread(length, 4, 1, file);
		if(res>0)
		{
			dataLen = bytesToInt(length,0);
			res = fread(data, dataLen, 1, file);
			
			ALOGTEST("startCodec------------3 res:%d dataLen:%d", res, dataLen);
			
			decorder(env, thiz, data, dataLen);
			
			usleep(50*1000);
		}
	} while (res>0);

	fclose(file);
}



static sp<VideoReceiver>	mpReceiveRender ;
static sp<FilesDecodec>		mpSenderRender	;
static sp<TestFileCodec>    mpFileDecoder;


jboolean JNI_API_NAME(StartVideoSend)(JNIEnv *env, jobject thiz, 						
						jobjectArray keys, jobjectArray values,
						jobject jsurface,
						jobject jcrypto,
						jint flags,
						jobject bufferInfo
						)
{
	ALOGE("Enter:StartVideoSend----------->1");
	
	sp<AMessage> format;
	status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

	sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpSenderRender = new FilesDecodec();
	ALOGE("Enter:StartVideoSend----------->2");
	mpSenderRender->CreateCodec(format, surface, crypto, flags);
	ALOGE("Enter:StartVideoSend----------->3");


	ALOGE("Enter:StartVideoSend----------->4");
	mpSenderRender->StartVideo(0);
	ALOGE("Enter:StartVideoSend----------->5");


	return true;
}

jboolean JNI_API_NAME(StopVideoSend)(JNIEnv *env, jobject)
{
	mpSenderRender->StopVideo();
	return mpSenderRender->DeInit();
}


jboolean JNI_API_NAME(StartFileDecoder)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags)
{
		ALOGE("Enter:StartVideoSend----------->1");
	
	sp<AMessage> format;
	status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

	sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpFileDecoder = new TestFileCodec();
	ALOGE("Enter:StartVideoSend----------->2");
	mpFileDecoder->CreateCodec(format, surface, crypto, flags);
	ALOGE("Enter:StartVideoSend----------->3");


	ALOGE("Enter:StartVideoSend----------->4");
	mpFileDecoder->StartVideo(0);
	ALOGE("Enter:StartVideoSend----------->5");
	
	return true;
}

jboolean JNI_API_NAME(StopFileDecoder)(JNIEnv *env, jobject)
{
	mpFileDecoder->StopVideo();
	return mpFileDecoder->DeInit();
}


jboolean JNI_API_NAME(StartVideoRecv)(JNIEnv *env, jobject thiz, jshort localRecvPort)
{
	return true;
}

jboolean JNI_API_NAME(StopVideoRecv)(JNIEnv *env, jobject)
{
	return true;
}


#ifdef __cplusplus
}
#endif






static JNINativeMethod gMethods[] = 
{
	
    { "native_configure",
      "([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      "Landroid/media/MediaCrypto;I)V",
      (void *)JNI_API_NAME(native_configure) },

	{ "startCodec", "(Landroid/media/MediaCodec$BufferInfo;)V", (void *)JNI_API_NAME(startCodec) },
	 
	{ "StartVideoSend", 
	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      "Landroid/media/MediaCrypto;ILandroid/media/MediaCodec$BufferInfo;)Z",
	(void *)JNI_API_NAME(StartVideoSend) },
	
	{ "StopVideoSend", "()Z", (void *)JNI_API_NAME(StopVideoSend) },

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
	
	ALOGTEST( "loading . . .2");
	
	result = JNI_VERSION_1_4;
	char prop[PROPERTY_VALUE_MAX] = "000";
	if(property_get("prop_name", prop, NULL) != 0)
	{
	}
	
	return result;
}

int register_Java_com_great_happyness_Codec_CodecMedia(JNIEnv *env) 
{
    return AndroidRuntime::registerNativeMethods(env, "Java/com/great/happyness/Codec/CodecMedia", gMethods, NELEM(gMethods));
}


