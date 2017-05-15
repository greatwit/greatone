#include <utils/Log.h>

#include "android_media_MediaCodec.h"

#include "android_media_MediaCrypto.h"
#include "android_media_Utils.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include "jni.h"
#include "JNIHelp.h" 

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




// Keep these in sync with their equivalents in MediaCodec.java !!!
enum {
    DEQUEUE_INFO_TRY_AGAIN_LATER            = -1, 
    DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED      = -2,
    DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED     = -3,
};

struct CryptoErrorCodes {
    jint cryptoErrorNoKey;
    jint cryptoErrorKeyExpired;
    jint cryptoErrorResourceBusy;
} gCryptoErrorCodes;

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

static sp<JMediaCodec> getMediaCodec(JNIEnv *env, jobject thiz) {
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
static void android_media_MediaCodec_native_configure(
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
        crypto = JCrypto::GetCrypto(env, jcrypto);
    }

    err = codec->configure(format, bufferProducer, crypto, flags);

    throwExceptionAsNecessary(env, err);
}

static void android_media_MediaCodec_native_setup(
        JNIEnv *env, jobject thiz,
        jstring name, jboolean nameIsType, jboolean encoder) {
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

static void android_media_MediaCodec_native_finalize(
        JNIEnv *env, jobject thiz) {
    android_media_MediaCodec_release(env, thiz);
}

static jobject android_media_MediaCodec_createInputSurface(JNIEnv* env,
        jobject thiz) {
    ALOGV("android_media_MediaCodec_createInputSurface");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);
    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NULL;
    }

    // Tell the MediaCodec that we want to use a Surface as input.
    sp<IGraphicBufferProducer> bufferProducer;
    status_t err = codec->createInputSurface(&bufferProducer);
    if (err != NO_ERROR) {
        throwExceptionAsNecessary(env, err);
        return NULL;
    }

    // Wrap the IGBP in a Java-language Surface.
    return android_view_Surface_createFromIGraphicBufferProducer(env,
            bufferProducer);
}

static void android_media_MediaCodec_release(JNIEnv *env, jobject thiz) {
    setMediaCodec(env, thiz, NULL);
}

static void android_media_MediaCodec_start(JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_start");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", "no codec found");
        return;
    }

    status_t err = codec->start();

    throwExceptionAsNecessary(env, err, "start failed");
}

static void android_media_MediaCodec_stop(JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_stop");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->stop();

    throwExceptionAsNecessary(env, err);
}

static void android_media_MediaCodec_flush(JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_flush");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->flush();

    throwExceptionAsNecessary(env, err);
}

static void android_media_MediaCodec_queueInputBuffer(
        JNIEnv *env,
        jobject thiz,
        jint index,
        jint offset,
        jint size,
        jlong timestampUs,
        jint flags) {
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

static void android_media_MediaCodec_queueSecureInputBuffer(
        JNIEnv *env,
        jobject thiz,
        jint index,
        jint offset,
        jobject cryptoInfoObj,
        jlong timestampUs,
        jint flags) {
    ALOGV("android_media_MediaCodec_queueSecureInputBuffer");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    jint numSubSamples =
        env->GetIntField(cryptoInfoObj, gFields.cryptoInfoNumSubSamplesID);

    jintArray numBytesOfClearDataObj =
        (jintArray)env->GetObjectField(
                cryptoInfoObj, gFields.cryptoInfoNumBytesOfClearDataID);

    jintArray numBytesOfEncryptedDataObj =
        (jintArray)env->GetObjectField(
                cryptoInfoObj, gFields.cryptoInfoNumBytesOfEncryptedDataID);

    jbyteArray keyObj =
        (jbyteArray)env->GetObjectField(cryptoInfoObj, gFields.cryptoInfoKeyID);

    jbyteArray ivObj =
        (jbyteArray)env->GetObjectField(cryptoInfoObj, gFields.cryptoInfoIVID);

    jint mode = env->GetIntField(cryptoInfoObj, gFields.cryptoInfoModeID);

    status_t err = OK;

    CryptoPlugin::SubSample *subSamples = NULL;
    jbyte *key = NULL;
    jbyte *iv = NULL;

    if (numSubSamples <= 0) {
        err = -EINVAL;
    } else if (numBytesOfClearDataObj == NULL
            && numBytesOfEncryptedDataObj == NULL) {
        err = -EINVAL;
    } else if (numBytesOfEncryptedDataObj != NULL
            && env->GetArrayLength(numBytesOfEncryptedDataObj) < numSubSamples) {
        err = -ERANGE;
    } else if (numBytesOfClearDataObj != NULL
            && env->GetArrayLength(numBytesOfClearDataObj) < numSubSamples) {
        err = -ERANGE;
    } else {
        jboolean isCopy;

        jint *numBytesOfClearData =
            (numBytesOfClearDataObj == NULL)
                ? NULL
                : env->GetIntArrayElements(numBytesOfClearDataObj, &isCopy);

        jint *numBytesOfEncryptedData =
            (numBytesOfEncryptedDataObj == NULL)
                ? NULL
                : env->GetIntArrayElements(numBytesOfEncryptedDataObj, &isCopy);

        subSamples = new CryptoPlugin::SubSample[numSubSamples];

        for (jint i = 0; i < numSubSamples; ++i) {
            subSamples[i].mNumBytesOfClearData =
                (numBytesOfClearData == NULL) ? 0 : numBytesOfClearData[i];

            subSamples[i].mNumBytesOfEncryptedData =
                (numBytesOfEncryptedData == NULL)
                    ? 0 : numBytesOfEncryptedData[i];
        }

        if (numBytesOfEncryptedData != NULL) {
            env->ReleaseIntArrayElements(
                    numBytesOfEncryptedDataObj, numBytesOfEncryptedData, 0);
            numBytesOfEncryptedData = NULL;
        }

        if (numBytesOfClearData != NULL) {
            env->ReleaseIntArrayElements(
                    numBytesOfClearDataObj, numBytesOfClearData, 0);
            numBytesOfClearData = NULL;
        }
    }

    if (err == OK && keyObj != NULL) {
        if (env->GetArrayLength(keyObj) != 16) {
            err = -EINVAL;
        } else {
            jboolean isCopy;
            key = env->GetByteArrayElements(keyObj, &isCopy);
        }
    }

    if (err == OK && ivObj != NULL) {
        if (env->GetArrayLength(ivObj) != 16) {
            err = -EINVAL;
        } else {
            jboolean isCopy;
            iv = env->GetByteArrayElements(ivObj, &isCopy);
        }
    }

    AString errorDetailMsg;

    if (err == OK) {
        err = codec->queueSecureInputBuffer(
                index, offset,
                subSamples, numSubSamples,
                (const uint8_t *)key, (const uint8_t *)iv,
                (CryptoPlugin::Mode)mode,
                timestampUs,
                flags,
                &errorDetailMsg);
    }

    if (iv != NULL) {
        env->ReleaseByteArrayElements(ivObj, iv, 0);
        iv = NULL;
    }

    if (key != NULL) {
        env->ReleaseByteArrayElements(keyObj, key, 0);
        key = NULL;
    }

    delete[] subSamples;
    subSamples = NULL;

    throwExceptionAsNecessary(
            env, err, errorDetailMsg.empty() ? NULL : errorDetailMsg.c_str());
}

static jint android_media_MediaCodec_dequeueInputBuffer(
        JNIEnv *env, jobject thiz, jlong timeoutUs) {
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

static jint android_media_MediaCodec_dequeueOutputBuffer(
        JNIEnv *env, jobject thiz, jobject bufferInfo, jlong timeoutUs) {
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

static void android_media_MediaCodec_releaseOutputBuffer(
        JNIEnv *env, jobject thiz, jint index, jboolean render) {
    ALOGV("android_media_MediaCodec_renderOutputBufferAndRelease");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->releaseOutputBuffer(index, render);

    throwExceptionAsNecessary(env, err);
}

static void android_media_MediaCodec_signalEndOfInputStream(JNIEnv* env,
        jobject thiz) {
    ALOGV("android_media_MediaCodec_signalEndOfInputStream");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);
    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    status_t err = codec->signalEndOfInputStream();

    throwExceptionAsNecessary(env, err);
}

static jobject android_media_MediaCodec_getOutputFormatNative(
        JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_getOutputFormatNative");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NULL;
    }

    jobject format;
    status_t err = codec->getOutputFormat(env, &format);

    if (err == OK) {
        return format;
    }

    throwExceptionAsNecessary(env, err);

    return NULL;
}

static jobjectArray android_media_MediaCodec_getBuffers(
        JNIEnv *env, jobject thiz, jboolean input) {
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

static jobject android_media_MediaCodec_getName(
        JNIEnv *env, jobject thiz) {
    ALOGV("android_media_MediaCodec_getName");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NULL;
    }

    jstring name;
    status_t err = codec->getName(env, &name);

    if (err == OK) {
        return name;
    }

    throwExceptionAsNecessary(env, err);

    return NULL;
}

static void android_media_MediaCodec_setParameters(
        JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray vals) {
    ALOGV("android_media_MediaCodec_setParameters");

    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    sp<AMessage> params;
    status_t err = ConvertKeyValueArraysToMessage(env, keys, vals, &params);

    if (err == OK) {
        err = codec->setParameters(params);
    }

    throwExceptionAsNecessary(env, err);
}

static void android_media_MediaCodec_setVideoScalingMode(
        JNIEnv *env, jobject thiz, jint mode) {
    sp<JMediaCodec> codec = getMediaCodec(env, thiz);

    if (codec == NULL) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if (mode != NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW
            && mode != NATIVE_WINDOW_SCALING_MODE_SCALE_CROP) {
        jniThrowException(env, "java/lang/InvalidArgumentException", NULL);
        return;
    }

    codec->setVideoScalingMode(mode);
}

static void android_media_MediaCodec_native_init(JNIEnv *env) {
    ScopedLocalRef<jclass> clazz( env, env->FindClass("android/media/MediaCodec"));
    CHECK(clazz.get() != NULL);

    gFields.context = env->GetFieldID(clazz.get(), "mNativeContext", "I");
    CHECK(gFields.context != NULL);

    clazz.reset(env->FindClass("android/media/MediaCodec$CryptoInfo"));
    CHECK(clazz.get() != NULL);

    gFields.cryptoInfoNumSubSamplesID =
        env->GetFieldID(clazz.get(), "numSubSamples", "I");
    CHECK(gFields.cryptoInfoNumSubSamplesID != NULL);

    gFields.cryptoInfoNumBytesOfClearDataID =
        env->GetFieldID(clazz.get(), "numBytesOfClearData", "[I");
    CHECK(gFields.cryptoInfoNumBytesOfClearDataID != NULL);

    gFields.cryptoInfoNumBytesOfEncryptedDataID =
        env->GetFieldID(clazz.get(), "numBytesOfEncryptedData", "[I");
    CHECK(gFields.cryptoInfoNumBytesOfEncryptedDataID != NULL);

    gFields.cryptoInfoKeyID = env->GetFieldID(clazz.get(), "key", "[B");
    CHECK(gFields.cryptoInfoKeyID != NULL);

    gFields.cryptoInfoIVID = env->GetFieldID(clazz.get(), "iv", "[B");
    CHECK(gFields.cryptoInfoIVID != NULL);

    gFields.cryptoInfoModeID = env->GetFieldID(clazz.get(), "mode", "I");
    CHECK(gFields.cryptoInfoModeID != NULL);

    clazz.reset(env->FindClass("android/media/MediaCodec$CryptoException"));
    CHECK(clazz.get() != NULL);

    jfieldID field;
    field = env->GetStaticFieldID(clazz.get(), "ERROR_NO_KEY", "I");
    CHECK(field != NULL);
    gCryptoErrorCodes.cryptoErrorNoKey =
        env->GetStaticIntField(clazz.get(), field);

    field = env->GetStaticFieldID(clazz.get(), "ERROR_KEY_EXPIRED", "I");
    CHECK(field != NULL);
    gCryptoErrorCodes.cryptoErrorKeyExpired =
        env->GetStaticIntField(clazz.get(), field);

    field = env->GetStaticFieldID(clazz.get(), "ERROR_RESOURCE_BUSY", "I");
    CHECK(field != NULL);
    gCryptoErrorCodes.cryptoErrorResourceBusy =
        env->GetStaticIntField(clazz.get(), field);
}



static JNINativeMethod gMethods[] = {
    { "release", "()V", (void *)android_media_MediaCodec_release },

    { "native_configure",
      "([Ljava/lang/String;[Ljava/lang/Object;Landroid/view/Surface;"
      "Landroid/media/MediaCrypto;I)V",
      (void *)android_media_MediaCodec_native_configure },

    { "createInputSurface", "()Landroid/view/Surface;",
      (void *)android_media_MediaCodec_createInputSurface },

    { "start", "()V", (void *)android_media_MediaCodec_start },
    { "stop", "()V", (void *)android_media_MediaCodec_stop },
    { "flush", "()V", (void *)android_media_MediaCodec_flush },

    { "queueInputBuffer", "(IIIJI)V",
      (void *)android_media_MediaCodec_queueInputBuffer },

    { "queueSecureInputBuffer", "(IILandroid/media/MediaCodec$CryptoInfo;JI)V",
      (void *)android_media_MediaCodec_queueSecureInputBuffer },

    { "dequeueInputBuffer", "(J)I",
      (void *)android_media_MediaCodec_dequeueInputBuffer },

    { "dequeueOutputBuffer", "(Landroid/media/MediaCodec$BufferInfo;J)I",
      (void *)android_media_MediaCodec_dequeueOutputBuffer },

    { "releaseOutputBuffer", "(IZ)V",
      (void *)android_media_MediaCodec_releaseOutputBuffer },

    { "signalEndOfInputStream", "()V",
      (void *)android_media_MediaCodec_signalEndOfInputStream },

    { "getOutputFormatNative", "()Ljava/util/Map;",
      (void *)android_media_MediaCodec_getOutputFormatNative },

    { "getBuffers", "(Z)[Ljava/nio/ByteBuffer;",
      (void *)android_media_MediaCodec_getBuffers },

    { "getName", "()Ljava/lang/String;",
      (void *)android_media_MediaCodec_getName },

    { "setParameters", "([Ljava/lang/String;[Ljava/lang/Object;)V",
      (void *)android_media_MediaCodec_setParameters },

    { "setVideoScalingMode", "(I)V",
      (void *)android_media_MediaCodec_setVideoScalingMode },

    { "native_init", "()V", (void *)android_media_MediaCodec_native_init },

    { "native_setup", "(Ljava/lang/String;ZZ)V",
      (void *)android_media_MediaCodec_native_setup },

    { "native_finalize", "()V",
      (void *)android_media_MediaCodec_native_finalize },
};

int register_android_media_MediaCodec(JNIEnv *env) {
    return AndroidRuntime::registerNativeMethods(env,
                "android/media/MediaCodec", gMethods, NELEM(gMethods));
}
*/








