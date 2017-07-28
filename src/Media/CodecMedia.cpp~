

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

#include "media_Utils.h"

#include "TestFileCodec.h"
#include "TestEncoder.h"

#include "CodecReceiver.h"
#include "CodecSender.h"


#include "CodecMedia.h"
#define TAG "CodecMedia"


char *mFilePath = "/sdcard/camera.h264";


using namespace android;

#ifdef __cplusplus
extern "C" 
{
#endif



uint8_t*bufferPoint[2];
//sp<JMediaCodec>  mCodec = NULL;


void JNI_API_NAME(native_configure)( JNIEnv *env, jobject thiz,
										jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags)
{
/*
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
	*/
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
	/*
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
	*/
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
			
			ALOGTEST("startCodec-----------3 res:%d dataLen:%d", res, dataLen);
			
			decorder(env, thiz, data, dataLen);
			
			usleep(50*1000);
		}
	} while (res>0);

	fclose(file);
}




static sp<TestFileCodec>    mpFileDecoder;
static TestEncoder          *mpFileEncoder = NULL;

 
static CodecReceiver *mpCodecRecv = NULL;
static CodecSender   *mpCodecSend = NULL;

jboolean JNI_API_NAME(StartVideoSend)(JNIEnv *env, jobject thiz, 						
						jobjectArray keys, jobjectArray values,
						jobject jsurface,
						jobject jcrypto,
						jint flags,
						jobject bufferInfo
						)
{
	ALOGE("Enter:StartVideoSend----------->1");
	/*
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
	*/

	return true;
}

jboolean JNI_API_NAME(StopVideoSend)(JNIEnv *env, jobject)
{
	//mpSenderRender->StopVideo();
	//mpSenderRender->DeInit();
	return true;
}


jboolean JNI_API_NAME(StartFileDecoder)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags)
{
		ALOGE("Enter:StartFileDecoder----------->1");
	
	sp<AMessage> format;
	status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

	sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpFileDecoder = new TestFileCodec();
	ALOGE("Enter:StartFileDecoder----------->2");
	mpFileDecoder->CreateCodec(format, surface, crypto, flags);
	ALOGE("Enter:StartFileDecoder----------->3");


	ALOGE("Enter:StartFileDecoder----------->4");
	mpFileDecoder->StartVideo(0);
	ALOGE("Enter:StartFileDecoder----------->5");
	
	return true;
}

jboolean JNI_API_NAME(StopFileDecoder)(JNIEnv *env, jobject)
{
	mpFileDecoder->StopVideo();
	return mpFileDecoder->DeInit();
}


jboolean JNI_API_NAME(StartFileEncoder)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags)
{
	ALOGE("Enter:StartFileEncoder----------->1");
	
	sp<AMessage> format;
	status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

	

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpFileEncoder = new TestEncoder();
	ALOGE("Enter:StartFileEncoder----------->2");

	if(jsurface!=NULL)
	{
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		mpFileEncoder->CreateCodec(format, surface, crypto, flags);
		ALOGE("Enter:StartFileEncoder----------->31");
	}else
	{
		mpFileEncoder->CreateCodec(format, NULL, crypto, flags);
		ALOGE("Enter:StartFileEncoder----------->32");
	}



	ALOGE("Enter:StartFileEncoder----------->4");
	mpFileEncoder->StartVideo(0);
	ALOGE("Enter:StartFileEncoder----------->5");
	
	return true;
}

jboolean JNI_API_NAME(AddEncoderData)(JNIEnv *env, jobject, jbyteArray byteData, jint len)
{
	jbyte* cameraFrame = env->GetByteArrayElements(byteData, NULL);
	mpFileEncoder->AddDecodecSource(reinterpret_cast<char*>(cameraFrame), len);
	env->ReleaseByteArrayElements(byteData, cameraFrame, JNI_ABORT);
	return true;
}

jboolean JNI_API_NAME(StopFileEncoder)(JNIEnv *env, jobject)
{
	mpFileEncoder->StopVideo();
	return mpFileEncoder->DeInit();
}

jboolean JNI_API_NAME(StartCodecSender)(JNIEnv *env, jobject, 
					jobjectArray keys, jobjectArray values,
					jobject jsurface, jobject jcrypto, jstring destip, 
					jshort destport, jshort localport, jint flags)
{
	
	ALOGE("Enter:StartCodecSender----------->1");
	
	sp<AMessage> format;
	status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

	

	sp<ICrypto> crypto;
	if (jcrypto != NULL) {
	//crypto = JCrypto::GetCrypto(env, jcrypto);
	}
	
	mpCodecSend = new CodecSender();
	ALOGE("Enter:StartCodecSender----------->2");

	if(jsurface!=NULL)
	{
		sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
		mpCodecSend->CreateCodec(format, surface, crypto, flags, localport);
		ALOGE("Enter:StartCodecSender----------->31");
	}else
	{
		mpCodecSend->CreateCodec(format, NULL, crypto, flags, localport);
		ALOGE("Enter:StartCodecSender----------->32");
	}

	const char *filePath = env->GetStringUTFChars(destip, NULL);
	mpCodecSend->ConnectDest(filePath, destport);
	env->ReleaseStringUTFChars(destip, filePath);
	
	ALOGE("Enter:StartCodecSender----------->4");
	mpCodecSend->StartVideo(0);
	ALOGE("Enter:StartCodecSender----------->5");
	
        return true;
}

jboolean JNI_API_NAME(CodecSenderData)(JNIEnv *env, jobject, jbyteArray byteData, jint len)
{
	jbyte* cameraFrame = env->GetByteArrayElements(byteData, NULL);
	mpCodecSend->AddDecodecSource(reinterpret_cast<char*>(cameraFrame), len);
	env->ReleaseByteArrayElements(byteData, cameraFrame, JNI_ABORT);
	return true;
}

jboolean JNI_API_NAME(StopCodecSender)(JNIEnv *env, jobject)
{
	mpCodecSend->StopVideo();
	mpCodecSend->DeInit();
	delete mpCodecSend;
	mpCodecSend = NULL;
	return true;
}

jboolean JNI_API_NAME(StartCodecRecver)(JNIEnv *env, jobject, 
					jobjectArray keys, jobjectArray values,
					jobject jsurface, jobject jcrypto, jint flags,
					jshort recvport)
{
	ALOGE("Enter:StartCodecRecver----------->1");
	
	sp<AMessage> format;
	status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

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

jboolean JNI_API_NAME(StopCodecRecver)(JNIEnv *env, jobject)
{
	mpCodecRecv->StopVideo();
	mpCodecRecv->DeInit();
	delete mpCodecRecv;
	mpCodecRecv = NULL;
	return true;
}

#ifdef __cplusplus
}
#endif






static JNINativeMethod gMethods[] = 
{
	/*
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
	*/
    	{ "StartFileDecoder",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;I)Z",
      	(void *)JNI_API_NAME(StartFileDecoder) },

	{ "StopFileDecoder", "()Z", (void *)JNI_API_NAME(StopFileDecoder) },

    	{ "StartFileEncoder",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;I)Z",
      	(void *)JNI_API_NAME(StartFileEncoder) },

	{ "StopFileEncoder", "()Z", (void *)JNI_API_NAME(StopFileEncoder) },

	{ "AddEncoderData", "([BI)Z", (void *)JNI_API_NAME(AddEncoderData) },


	{ "StartCodecSender",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;Ljava/lang/String;SSI)Z",
      	(void *)JNI_API_NAME(StartCodecSender) },

	{ "StopCodecSender", "()Z", (void *)JNI_API_NAME(StopCodecSender) },

	{ "StartCodecRecver",
      	"([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      	"Landroid/media/MediaCrypto;IS)Z",
      	(void *)JNI_API_NAME(StartCodecRecver) },

	{ "StopCodecRecver", "()Z", (void *)JNI_API_NAME(StopCodecRecver) },
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


