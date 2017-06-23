

#include <utils/Log.h>

#include "GMediaCodec.h"
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


#include "VideoSender.h"
#include "VideoReceiver.h"

#include "CodecMedia.h"
#define TAG "CodecMedia"




#define PROPERTY_KEY_MAX    32
#define PROPERTY_VALUE_MAX  92

char *mFilePath = "/sdcard/camera.h264";


using namespace android;

#ifdef __cplusplus
extern "C" 
{
#endif

enum {
    DEQUEUE_INFO_TRY_AGAIN_LATER            = -1,
    DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED      = -2,
    DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED     = -3,
};

struct fields_t 
{
    jfieldID context;

    jfieldID cryptoInfoNumSubSamplesID;
    jfieldID cryptoInfoNumBytesOfClearDataID;
    jfieldID cryptoInfoNumBytesOfEncryptedDataID;
    jfieldID cryptoInfoKeyID;
    jfieldID cryptoInfoIVID;
    jfieldID cryptoInfoModeID;
};

static fields_t gFields;
uint8_t*bufferPoint[2];
jobject mBufferInfo;

sp<GMediaCodec>  mCodec = NULL;

static sp<GMediaCodec> setMediaCodec( JNIEnv *env, jobject thiz, const sp<GMediaCodec> &codec) 
{
	/*
    sp<JMediaCodec> old = (JMediaCodec *)env->GetIntField(thiz, gFields.context);
    if (codec != NULL) 
	{
        codec->incStrong(thiz);
    }
    if (old != NULL) 
	{
        old->decStrong(thiz);
    }
    env->SetIntField(thiz, gFields.context, (int)codec.get());
	*/
	mCodec = codec;
    return mCodec;
}

static sp<GMediaCodec> getMediaCodec(JNIEnv *env, jobject thiz) 
{
    return mCodec;//(JMediaCodec *)env->GetIntField(thiz, gFields.context);
}



static void throwCryptoException(JNIEnv *env, status_t err, const char *msg) 
{
    jclass clazz = env->FindClass("android/media/MediaCodec$CryptoException");
    CHECK(clazz != NULL);

    jmethodID constructID =
        env->GetMethodID(clazz, "<init>", "(ILjava/lang/String;)V");
    CHECK(constructID != NULL);

    jstring msgObj = env->NewStringUTF(msg != NULL ? msg : "Unknown Error");

    jthrowable exception =
        (jthrowable)env->NewObject(clazz, constructID, err, msgObj);

    env->Throw(exception);
}

static jint throwExceptionAsNecessary(
        JNIEnv *env, status_t err, const char *msg = NULL) 
{


#if HEIGHT_VERSION
    if (err >= ERROR_DRM_VENDOR_MIN && err <= ERROR_DRM_VENDOR_MAX) 
	{
        // We'll throw our custom MediaCodec.CryptoException
        throwCryptoException(env, err, msg);
        return 0;
    }
#else
    if (err >= ERROR_DRM_WV_VENDOR_MIN && err <= ERROR_DRM_WV_VENDOR_MAX) 
    {
        // We'll throw our custom MediaCodec.CryptoException

        throwCryptoException(env, err, msg);
        return 0;
    }
#endif


    switch (err) 
	{
        case OK:
            return 0;

        case -EAGAIN:
            return DEQUEUE_INFO_TRY_AGAIN_LATER;

        case INFO_FORMAT_CHANGED:
            return DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED;

        case INFO_OUTPUT_BUFFERS_CHANGED:
            return DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED;

        default:
        {
            jniThrowException(env, "java/lang/IllegalStateException", NULL);
            break;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////


void JNI_API_NAME(native_setup)(JNIEnv *env, jobject thiz, jstring name, jboolean nameIsType, jboolean encoder) 
{
	ALOGTEST("native_setup...");

	if (name == NULL)
	{
		jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
		return;
	}

	const char *tmp = env->GetStringUTFChars(name, NULL);

	if (tmp == NULL) {
		return;
	}

	sp<GMediaCodec> codec = new GMediaCodec( tmp, nameIsType, encoder);

	status_t err = codec->initCheck();

	env->ReleaseStringUTFChars(name, tmp);
	tmp = NULL;

	if (err != OK) 
	{
		jniThrowException(
			env,
			"java/io/IOException",
			"Failed to allocate component instance");
		return;
	}

	setMediaCodec(env,thiz, codec);
}

void JNI_API_NAME(native_configure)(
        JNIEnv *env,
        jobject thiz,
        jobjectArray keys, jobjectArray values,
        jobject jsurface,
        jobject jcrypto,
        jint flags)
{
    sp<GMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) 
	{
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    sp<AMessage> format;
    status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

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
	    if (jcrypto != NULL) 
		{
			crypto = JCrypto::GetCrypto(env, jcrypto);
	    }

	    err = codec->configure(format, bufferProducer, crypto, flags);
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
	    if (jcrypto != NULL) 
		{
			//crypto = JCrypto::GetCrypto(env, jcrypto);
	    }

	    err = codec->configure(format, surfaceTexture, crypto, flags);
	#endif

    throwExceptionAsNecessary(env, err);
}

void JNI_API_NAME(release)(JNIEnv *env, jobject thiz)
{
    setMediaCodec(env, thiz, NULL);
}

void JNI_API_NAME(start)(JNIEnv *env, jobject thiz)
{
    ALOGV("android_media_MediaCodec_start");

    sp<GMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) 
	{
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->start();

	jobjectArray inputBuffers;
	err = codec->getBuffers(true, &inputBuffers);

    throwExceptionAsNecessary(env, err);

}

void JNI_API_NAME(startCodec)( JNIEnv *env, jobject thiz, jobject bufferInfo)
{
	char length[4] = {0};
	char data[1000000] = {0};

    
	mBufferInfo = bufferInfo;
	
	sp<GMediaCodec> codec = getMediaCodec(env, thiz);
	codec->startCodec();
}

void JNI_API_NAME(native_init)(JNIEnv *env, jobject thiz)
{
	ALOGE("native_init----------->");
}


static JavaVM *g_JavaVM  = android::AndroidRuntime::getJavaVM(); 
static JNIEnv *GetEnv()
{  
	int status;  
    JNIEnv *envnow = NULL;  
    status = g_JavaVM->GetEnv((void **)&envnow, JNI_VERSION_1_4);  
    if(status < 0)  
    {  
        status = g_JavaVM->AttachCurrentThread(&envnow, NULL);
        if(status < 0)
        {
            return NULL; 
        }
    }  
    return envnow;
}

JavaVM *GJavaVM  = NULL;

class DataReceiver : public IReceiveCallback
{
	public:
		DataReceiver(JNIEnv *env, jobject thiz)
		{
			mEnv 	= env;
			mThiz 	= thiz;
		}
		~DataReceiver(){}
		void ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen)
		{
			ALOGE("TAG 4:%s,line=:%d, data length=:%d mimetype:%s ",__FUNCTION__,__LINE__, dataLen, mimeType);
	
			//decorder(mEnv, mThiz, (char*)buffer, dataLen);
			mCodec->addBuffer((char*)buffer, dataLen);
		}//
		
	private:
		JNIEnv *mEnv;
		jobject mThiz;
};



static sp<VideoReceiver>	mpReceiveRender ;
static sp<VideoSender>		mpSenderRender	;
static RtpReceive			*mpReceive = NULL;
static DataReceiver         *mpDataRecv = NULL;

jboolean JNI_API_NAME(StartVideoSend)(JNIEnv *env, jobject thiz, jstring destip, jshort localSendPort, jshort remotePort)
{
	ALOGE("Enter:StartVideoSend----------->1");
	mpSenderRender = new VideoSender();
	ALOGE("Enter:StartVideoSend----------->2");
	mpSenderRender->Init(NULL, 0, 0, localSendPort);
	ALOGE("Enter:StartVideoSend----------->3");
	const char *ip = env->GetStringUTFChars(destip, NULL);
	jsize nLength  = env->GetStringUTFLength(destip);
	ALOGV("Enter:AudioConnectDest----------->strIP:%s,nLength:%d",ip, nLength);

	//mpDataRecv = new DataReceiver(env, thiz);
	//mpSenderRender->registerCallback(mpDataRecv);
	std::string stIp = std::string(ip);
	mpSenderRender->ConnectDest(stIp, remotePort);
	ALOGE("Enter:StartVideoSend----------->4");
	mpSenderRender->StartVideo(0);
	ALOGE("Enter:StartVideoSend----------->5");
	env->ReleaseStringUTFChars(destip, ip);

	return true;
}

jboolean JNI_API_NAME(StopVideoSend)(JNIEnv *env, jobject)
{
	mpSenderRender->StopVideo();
	//SAFE_DELETE(mpDataRecv);
	mpSenderRender->DeInit();
	mpSenderRender = NULL;
	return true;
}


jboolean JNI_API_NAME(StartVideoRecv)(JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags, jshort localRecvPort)
{
	/*
	mpReceive = new RtpReceive();
	mpDataRecv = new DataReceiver(env, thiz);
	if(mpReceive->initSession(localRecvPort))
	{
		mpReceive->registerCallback(mpDataRecv);
		mpReceive->startThread();
	}
	*/
	ALOGW("StartVideoRecv----------->1");
	sp<AMessage> format;
    status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);
    if (err != OK) 
	{
        return false;
    }

	if (jsurface != NULL) 
	{
		sp<Surface> surf(android_view_Surface_getSurface(env, jsurface));
   
		sp<ICrypto> crypto;
		if (jcrypto != NULL) 
		{
			//crypto = JCrypto::GetCrypto(env, jcrypto);
		}

		mpReceiveRender = new VideoReceiver();
		mpReceiveRender->Init(format, surf, crypto, flags, localRecvPort);
		mpReceiveRender->StartVideo(0);
	}

	ALOGW("StartVideoRecv----------->2");
	
	return true;
}

jboolean JNI_API_NAME(StopVideoRecv)(JNIEnv *env, jobject)
{
	/*
	mpReceive->stopThread();
	mpReceive->deinitSession();
	SAFE_DELETE(mpReceive);
	SAFE_DELETE(mpDataRecv);
	*/
	mpReceiveRender->StopVideo();
	mpReceiveRender->DeInit();
	mpReceiveRender = NULL;
	return true;
}


#ifdef __cplusplus
}
#endif


static JNINativeMethod gMethods[] = 
{
    { "release", "()V", (void *)JNI_API_NAME(release) },

    { "native_configure",
      "([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      "Landroid/media/MediaCrypto;I)V",
      (void *)JNI_API_NAME(native_configure) },

    { "start", "()V", (void *)JNI_API_NAME(start) },

	{ "startCodec", "(Landroid/media/MediaCodec$BufferInfo;)V", (void *)JNI_API_NAME(startCodec) },

    { "native_init", "()V", (void *)JNI_API_NAME(native_init) },

    { "native_setup", "(Ljava/lang/String;ZZ)V", (void *)JNI_API_NAME(native_setup) },
	 
	{ "StartVideoSend", "(Ljava/lang/String;SS)Z", (void *)JNI_API_NAME(StartVideoSend) },
	{ "StopVideoSend", "()Z", (void *)JNI_API_NAME(StopVideoSend) },
	{ "StartVideoRecv", 
	  "([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      "Landroid/media/MediaCrypto;IS)Z",
	(void *)JNI_API_NAME(StartVideoRecv) },
	{ "StopVideoRecv", "()Z", (void *)JNI_API_NAME(StopVideoRecv) },
	
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
	
	GJavaVM = vm;
	
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


