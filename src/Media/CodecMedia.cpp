


#define LOG_TAG "MediaCodec-JNI"

#include <utils/Log.h>

#include "android_media_MediaCodec.h"

#include "android_media_MediaCrypto.h"
#include "android_media_Utils.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "jni.h"
#include "JNIHelp.h"

#include "cutils/properties.h"

#include <gui/Surface.h>
#include <gui/SurfaceTextureClient.h>

#include <media/ICrypto.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>

#include <system/window.h>

#define JNI_API_NAME(A)		Java_com_great_happyness_Codec_CodecMedia_##A 

#define TAG "CodecMedia"

#define ALOGTEST(...)  __android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)

#define PROPERTY_KEY_MAX    32
#define PROPERTY_VALUE_MAX  92

char *mFilePath = "/sdcard/camera.h264";


using namespace android;


// Keep these in sync with their equivalents in MediaCodec.java !!!
enum {
    DEQUEUE_INFO_TRY_AGAIN_LATER            = -1,
    DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED      = -2,
    DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED     = -3,
};

struct fields_t {
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


static sp<JMediaCodec> setMediaCodec(
        JNIEnv *env, jobject thiz, const sp<JMediaCodec> &codec) {
    sp<JMediaCodec> old = (JMediaCodec *)env->GetIntField(thiz, gFields.context);
    if (codec != NULL) {
        codec->incStrong(thiz);
    }
    if (old != NULL) {
        old->decStrong(thiz);
    }
    env->SetIntField(thiz, gFields.context, (int)codec.get());

    return old;
}

static sp<JMediaCodec> getMediaCodec(JNIEnv *env, jobject thiz) 
{
    return (JMediaCodec *)env->GetIntField(thiz, gFields.context);
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
    if (err >= ERROR_DRM_WV_VENDOR_MIN && err <= ERROR_DRM_WV_VENDOR_MAX) 
	{
        // We'll throw our custom MediaCodec.CryptoException

        throwCryptoException(env, err, msg);
        return 0;
    }

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

static void JNI_API_NAME(native_configure)(
        JNIEnv *env,
        jobject thiz,
        jobjectArray keys, jobjectArray values,
        jobject jsurface,
        jobject jcrypto,
        jint flags) 
{
    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    sp<AMessage> format;
    status_t err = ConvertKeyValueArraysToMessage(env, keys, values, &format);

    if (err != OK) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    sp<ISurfaceTexture> surfaceTexture;
    if (jsurface != NULL) {
        sp<Surface> surface(android_view_Surface_getSurface(env, jsurface));
        if (surface != NULL) {
            surfaceTexture = surface->getSurfaceTexture();
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
        crypto = JCrypto::GetCrypto(env, jcrypto);
    }

    err = codec->configure(format, surfaceTexture, crypto, flags);


    throwExceptionAsNecessary(env, err);
}

static void JNI_API_NAME(release)(JNIEnv *env, jobject thiz) 
{
    setMediaCodec(env, thiz, NULL);
}

static void JNI_API_NAME(start)(JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_start");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->start();

	jobjectArray inputBuffers;
	err = codec->getBuffers(env, true, &inputBuffers);

	for(int i =0;i<2;i++)
	{
		jobject inputObject = env->GetObjectArrayElement(inputBuffers, i);
		bufferPoint[i] 	= (uint8_t*)env->GetDirectBufferAddress( inputObject);
	}
	
    throwExceptionAsNecessary(env, err);
}

static void JNI_API_NAME(stop)(JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_stop");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->stop();

    throwExceptionAsNecessary(env, err);
}

static void JNI_API_NAME(flush)(JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_flush");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->flush();

    throwExceptionAsNecessary(env, err);
}


jobjectArray mInputBuffers;
jobjectArray mOutputBuffers;
// env->DeleteGlobalRef( p_sys->input_buffers);


static jobjectArray JNI_API_NAME(getBuffers)(
        JNIEnv *env, jobject thiz, jboolean input) 
{
    ALOGV("android_media_MediaCodec_getBuffers");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NULL;
    }

	
    jobjectArray buffers;
    status_t err = codec->getBuffers(env, input, &buffers);

    if (err == OK) {
        return buffers;
    }
	/*
	status_t err;
	if(input)
	{
		err = codec->getBuffers(env, input, &mInputBuffers);
		if (err == OK)
			return mInputBuffers;
	}
	else
	{
		err = codec->getBuffers(env, input, &mOutputBuffers);
		if (err == OK)
			return mOutputBuffers;
	}
	*/
    throwExceptionAsNecessary(env, err);

    return NULL;
}

static void JNI_API_NAME(queueInputBuffer)(
        JNIEnv *env,
        jobject thiz,
        jint index,
        jint offset,
        jint size,
        jlong timestampUs,
        jint flags) 
{
    ALOGV("android_media_MediaCodec_queueInputBuffer");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    AString errorDetailMsg;

    status_t err = codec->queueInputBuffer(
            index, offset, size, timestampUs, flags, &errorDetailMsg);

    throwExceptionAsNecessary(
            env, err, errorDetailMsg.empty() ? NULL : errorDetailMsg.c_str());
}


static jint JNI_API_NAME(dequeueInputBuffer)(
        JNIEnv *env, jobject thiz, jlong timeoutUs) 
{
    ALOGV("android_media_MediaCodec_dequeueInputBuffer");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }

    size_t index;
    status_t err = codec->dequeueInputBuffer(&index, timeoutUs);

    if (err == OK) {
        return index;
    }

    return throwExceptionAsNecessary(env, err);
}

static jint JNI_API_NAME(dequeueOutputBuffer)(
        JNIEnv *env, jobject thiz, jobject bufferInfo, jlong timeoutUs) 
{
    ALOGV("android_media_MediaCodec_dequeueOutputBuffer");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return 0;
    }

    size_t index;
    status_t err = codec->dequeueOutputBuffer(
            env, bufferInfo, &index, timeoutUs);

    if (err == OK) {
        return index;
    }

    return throwExceptionAsNecessary(env, err);
}

static void JNI_API_NAME(releaseOutputBuffer)(
        JNIEnv *env, jobject thiz, jint index, jboolean render) 
{
    ALOGV("android_media_MediaCodec_renderOutputBufferAndRelease");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->releaseOutputBuffer(index, render);

    throwExceptionAsNecessary(env, err);
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
	sp<JMediaCodec> codec = getMediaCodec(env, thiz);

	ALOGTEST("startCodec------------1");
	
    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return ;
    }
	
	size_t inputBufferIndex = 0, outputBufferIndex = 0;
	
	status_t err;
	int mCount = 0;
	AString errorDetailMsg;
	
	
	err = codec->dequeueInputBuffer(&inputBufferIndex, -1);
	ALOGTEST("startCodec------------4 err:%d", err);


	uint8_t* inputChar = bufferPoint[inputBufferIndex];
	memcpy(inputChar, data, dataLen);
	
	err = codec->queueInputBuffer(inputBufferIndex, 0, dataLen, mCount * 1000000 / 20, 0, &errorDetailMsg);
	mCount++;
	
	ALOGTEST("startCodec------------6 err:%d input index:%d inputChar addr:%d", err, inputBufferIndex, inputChar);
	
	
	err = codec->dequeueOutputBuffer(env, mBufferInfo, &outputBufferIndex, 0);
	ALOGTEST("startCodec------------7 err:%d outputBufferIndex:%d", err, outputBufferIndex);
	
	while (err ==0 && outputBufferIndex >= 0) 
	{
			codec->releaseOutputBuffer(outputBufferIndex, true);
			err = codec->dequeueOutputBuffer(env, mBufferInfo, &outputBufferIndex, 0);
	}

	if (outputBufferIndex < 0) 
	{
		ALOGTEST("startCodec------------err :%d ", outputBufferIndex);
	}
	
	ALOGTEST("startCodec------------8");
}

static void JNI_API_NAME(startCodec)( JNIEnv *env, jobject thiz, jobject bufferInfo)
{

	
	char length[4] = {0};
	char data[1000000] = {0};

	FILE *file = fopen(mFilePath, "rb");
	int res = 0, dataLen = 0;

    
	mBufferInfo = bufferInfo;
	
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


static void JNI_API_NAME(native_init)(JNIEnv *env) 
{
    jclass clazz = env->FindClass("android/media/MediaCodec");
    CHECK(clazz != NULL);

    gFields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    CHECK(gFields.context != NULL);

    clazz = env->FindClass("android/media/MediaCodec$CryptoInfo");
    CHECK(clazz != NULL);

    gFields.cryptoInfoNumSubSamplesID =
        env->GetFieldID(clazz, "numSubSamples", "I");
    CHECK(gFields.cryptoInfoNumSubSamplesID != NULL);

    gFields.cryptoInfoNumBytesOfClearDataID =
        env->GetFieldID(clazz, "numBytesOfClearData", "[I");
    CHECK(gFields.cryptoInfoNumBytesOfClearDataID != NULL);

    gFields.cryptoInfoNumBytesOfEncryptedDataID =
        env->GetFieldID(clazz, "numBytesOfEncryptedData", "[I");
    CHECK(gFields.cryptoInfoNumBytesOfEncryptedDataID != NULL);

    gFields.cryptoInfoKeyID = env->GetFieldID(clazz, "key", "[B");
    CHECK(gFields.cryptoInfoKeyID != NULL);

    gFields.cryptoInfoIVID = env->GetFieldID(clazz, "iv", "[B");
    CHECK(gFields.cryptoInfoIVID != NULL);

    gFields.cryptoInfoModeID = env->GetFieldID(clazz, "mode", "I");
    CHECK(gFields.cryptoInfoModeID != NULL);
}

static void JNI_API_NAME(native_setup)(
        JNIEnv *env, jobject thiz,
        jstring name, jboolean nameIsType, jboolean encoder) 
{
	__android_log_print(ANDROID_LOG_INFO, TAG, "native_setup . . .");
	
    if (name == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *tmp = env->GetStringUTFChars(name, NULL);

    if (tmp == NULL) {
        return;
    }

    sp<JMediaCodec> codec = new JMediaCodec(env, thiz, tmp, nameIsType, encoder);

    status_t err = codec->initCheck();

    env->ReleaseStringUTFChars(name, tmp);
    tmp = NULL;

    if (err != OK) {
        jniThrowException(
                env,
                "java/io/IOException",
                "Failed to allocate component instance");
        return;
    }

    setMediaCodec(env,thiz, codec);
}


static JNINativeMethod gMethods[] = {
    { "release", "()V", (void *)JNI_API_NAME(release) },

    { "native_configure",
      "([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      "Landroid/media/MediaCrypto;I)V",
      (void *)JNI_API_NAME(native_configure) },

    { "start", "()V", (void *)JNI_API_NAME(start) },
    { "stop", "()V", (void *)JNI_API_NAME(stop) },
    { "flush", "()V", (void *)JNI_API_NAME(flush) },
	{ "startCodec", "(Landroid/media/MediaCodec$BufferInfo;)V", (void *)JNI_API_NAME(startCodec) },

    { "queueInputBuffer", "(IIIJI)V",
      (void *)JNI_API_NAME(queueInputBuffer) },

    { "dequeueInputBuffer", "(J)I",
      (void *)JNI_API_NAME(dequeueInputBuffer) },

    { "dequeueOutputBuffer", "(Landroid/media/MediaCodec$BufferInfo;J)I",
      (void *)JNI_API_NAME(dequeueOutputBuffer) },

    { "releaseOutputBuffer", "(IZ)V",
      (void *)JNI_API_NAME(releaseOutputBuffer) },
	  
    { "getBuffers", "(Z)[Ljava/nio/ByteBuffer;",
      (void *)JNI_API_NAME(getBuffers) },

    { "native_init", "()V", (void *)JNI_API_NAME(native_init) },

    { "native_setup", "(Ljava/lang/String;ZZ)V",
      (void *)JNI_API_NAME(native_setup) },
	  
};

int jniRegisterNativeMethods1(JNIEnv* env,
							 const char* className,
							 const JNINativeMethod* methods,
							 int numMethods)
{
	jclass clazz;

	__android_log_print(ANDROID_LOG_INFO, TAG, "Registering %s natives\n", className);
	clazz = env->FindClass(className);
	__android_log_print(ANDROID_LOG_INFO, TAG, "Registering %s natives 1\n", className);
	if (clazz == NULL) 
	{
		__android_log_print(ANDROID_LOG_ERROR, TAG, "Native registration unable to find class '%s'\n", className);
		return -1;
	}
	__android_log_print(ANDROID_LOG_INFO, TAG, "Registering %s natives 2\n", className);
	if (env->RegisterNatives(clazz, methods, numMethods) < 0) 
	{
		__android_log_print(ANDROID_LOG_ERROR, TAG, "RegisterNatives failed for '%s'\n", className);
		return -1;
	}
	__android_log_print(ANDROID_LOG_INFO, TAG, "Registering %s natives 3\n", className);
	return 0;
}

int registerNatives(JNIEnv *env) {
	return jniRegisterNativeMethods1(env, "com/great/happyness/Codec/CodecMedia", gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = JNI_ERR;

	__android_log_print(ANDROID_LOG_INFO, TAG, "loading . . .1");
	
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) 
	{
		__android_log_print(ANDROID_LOG_ERROR, TAG, "GetEnv failed!");
		return result;
	}

	
	if( registerNatives(env) != JNI_OK) 
	{
		__android_log_print(ANDROID_LOG_ERROR, TAG, "can't load ffmpeg");
	}
	
	__android_log_print(ANDROID_LOG_INFO, TAG, "loading . . .2");
	
	result = JNI_VERSION_1_4;
	char prop[PROPERTY_VALUE_MAX] = "000";
if(property_get("prop_name", prop, NULL) != 0)
{
	
}
	return result;
}

/*
int register_android_media_MediaCodec(JNIEnv *env) {
    return AndroidRuntime::registerNativeMethods(env, "android/media/MediaCodec", gMethods, NELEM(gMethods));
}
*/
int register_Java_com_great_happyness_Codec_CodecMedia(JNIEnv *env) {
    return AndroidRuntime::registerNativeMethods(env, "Java/com/great/happyness/Codec/CodecMedia", gMethods, NELEM(gMethods));
}

