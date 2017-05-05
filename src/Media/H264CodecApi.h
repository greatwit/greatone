#ifndef _H264_CODEC_API_H_
#define _H264_CODEC_API_H_

#define JNI_API_NAME(A)		Java_com_great_##A


#ifdef __cplusplus
extern "C"
{
#endif


JNIEXPORT void JNICALL JNI_API_NAME(release)(JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL JNI_API_NAME(native_configure)( JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags);
JNIEXPORT jobject JNICALL JNI_API_NAME(createInputSurface)(JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL JNI_API_NAME(start)(JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL JNI_API_NAME(stop)(JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL JNI_API_NAME(flush)(JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL JNI_API_NAME(queueInputBuffer)( JNIEnv *env, jobject thiz, jint index, jint offset, jint size, jlong timestampUs, jint flags);
JNIEXPORT void JNICALL JNI_API_NAME(queueSecureInputBuffer)(JNIEnv *env, jobject thiz, jint index, jint offset, jobject cryptoInfoObj, jlong timestampUs, jint flags);
JNIEXPORT jint JNICALL JNI_API_NAME(dequeueInputBuffer)( JNIEnv *env, jobject thiz, jlong timeoutUs);
JNIEXPORT jint JNICALL JNI_API_NAME(dequeueOutputBuffer)( JNIEnv *env, jobject thiz, jobject bufferInfo, jlong timeoutUs);
JNIEXPORT void JNICALL JNI_API_NAME(releaseOutputBuffer)( JNIEnv *env, jobject thiz, jint index, jboolean render);
JNIEXPORT void JNICALL JNI_API_NAME(signalEndOfInputStream)(JNIEnv* env, jobject thiz);
JNIEXPORT jobject JNICALL JNI_API_NAME(getOutputFormatNative)( JNIEnv *env, jobject thiz);
JNIEXPORT jobject JNICALL JNI_API_NAME(getBuffers)( JNIEnv *env, jobject thiz, jboolean input);
JNIEXPORT jobject JNICALL JNI_API_NAME(getName)( JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL JNI_API_NAME(setParameters)( JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray vals);
JNIEXPORT void JNICALL JNI_API_NAME(setVideoScalingMode)( JNIEnv *env, jobject thiz, jint mode);
JNIEXPORT void JNICALL JNI_API_NAME(native_setup)( JNIEnv *env, jobject thiz, jstring name, jboolean nameIsType, jboolean encoder);


#ifdef __cplusplus
}
#endif


#endif
