#include <utils/Log.h>

#include "android_media_MediaCodec.h"

#include "android_media_MediaCrypto.h"
#include "android_media_Utils.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "jni.h"


#include <gui/Surface.h>

#include <media/ICrypto.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>

//#include <nativehelper/ScopedLocalRef.h>

#include <system/window.h> 
#include "H264CodecApi.h"

using namespace android;


#ifdef __cplusplus
extern "C" 
{
#endif


// Keep these in sync with their equivalents in MediaCodec.java !!!
enum 
{
    DEQUEUE_INFO_TRY_AGAIN_LATER            = -1, 
    DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED      = -2,
    DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED     = -3,
};

struct CryptoErrorCodes 
{
    jint cryptoErrorNoKey;
    jint cryptoErrorKeyExpired;
    jint cryptoErrorResourceBusy;
} gCryptoErrorCodes;

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

static sp<JMediaCodec> setMediaCodec(
    JNIEnv *env, jobject thiz, const sp<JMediaCodec> &codec) 
{
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

    return old;
}

static sp<JMediaCodec> getMediaCodec(JNIEnv *env, jobject thiz) 
{
    return (JMediaCodec *)env->GetIntField(thiz, gFields.context);
}

static void android_media_MediaCodec_release(JNIEnv *env, jobject thiz) {
    setMediaCodec(env, thiz, NULL);
}

static void throwCryptoException(JNIEnv *env, status_t err, const char *msg) {
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
        JNIEnv *env, status_t err, const char *msg = NULL) {
    if (err >= ERROR_DRM_WV_VENDOR_MIN && err <= ERROR_DRM_WV_VENDOR_MAX) {
        // We'll throw our custom MediaCodec.CryptoException

        throwCryptoException(env, err, msg);
        return 0;
    }

    switch (err) {
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

void JNI_API_NAME(native_setup)( JNIEnv *env, jobject thiz, jstring name, jboolean nameIsType, jboolean encoder)
{
	/*
	if (name == NULL) 
	{
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *tmp = env->GetStringUTFChars(name, NULL);

    if (tmp == NULL) 
	{
        return;
    }
	*/
	ALOGV("=============native_setup=================");
	/*
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
	*/
}

void JNI_API_NAME(release)(JNIEnv *env, jobject thiz)
{
	setMediaCodec(env, thiz, NULL);
}

void JNI_API_NAME(native_configure)( JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags)
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

void JNI_API_NAME(start)(JNIEnv *env, jobject thiz)
{
	ALOGV("android_media_MediaCodec_start");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->start();

    throwExceptionAsNecessary(env, err);
}

void JNI_API_NAME(stop)(JNIEnv *env, jobject thiz)
{
	ALOGV("android_media_MediaCodec_stop");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->stop();

    throwExceptionAsNecessary(env, err);
}

void JNI_API_NAME(flush)(JNIEnv *env, jobject thiz)
{
	ALOGV("android_media_MediaCodec_flush");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->flush();

    throwExceptionAsNecessary(env, err);
}

jobject JNI_API_NAME(getBuffers)( JNIEnv *env, jobject thiz, jboolean input)
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

    // if we're out of memory, an exception was already thrown
    if (err != NO_MEMORY) {
        throwExceptionAsNecessary(env, err);
    }

    return NULL;
}

jint JNI_API_NAME(dequeueInputBuffer)( JNIEnv *env, jobject thiz, jlong timeoutUs)
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

void JNI_API_NAME(queueInputBuffer)( JNIEnv *env, jobject thiz, jint index, jint offset, jint size, jlong timestampUs, jint flags)
{
	ALOGV("android_media_MediaCodec_queueInputBuffer");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) 
	{
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    AString errorDetailMsg;

    status_t err = codec->queueInputBuffer(index, offset, size, timestampUs, flags, &errorDetailMsg);

    throwExceptionAsNecessary(env, err, errorDetailMsg.empty() ? NULL : errorDetailMsg.c_str());
}

jint JNI_API_NAME(dequeueOutputBuffer)( JNIEnv *env, jobject thiz, jobject bufferInfo, jlong timeoutUs)
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

void JNI_API_NAME(releaseOutputBuffer)( JNIEnv *env, jobject thiz, jint index, jboolean render)
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



#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



