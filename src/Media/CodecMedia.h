#ifndef CODEC_MEDIA_HEAD_H_
#define CODEC_MEDIA_HEAD_H_



#define JNI_API_NAME(A)		Java_com_great_happyness_Codec_CodecMedia_##A 


#ifdef __cplusplus
extern "C"
{
#endif

	JNIEXPORT void JNICALL JNI_API_NAME(native_configure)( JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags);
	JNIEXPORT void JNICALL JNI_API_NAME(release)(JNIEnv *env, jobject thiz);
	JNIEXPORT void JNICALL JNI_API_NAME(start)(JNIEnv *env, jobject thiz);
	
	JNIEXPORT void JNICALL JNI_API_NAME(stop)(JNIEnv *env, jobject thiz);
	JNIEXPORT void JNICALL JNI_API_NAME(flush)(JNIEnv *env, jobject thiz);
	JNIEXPORT jobjectArray JNICALL JNI_API_NAME(getBuffers)( JNIEnv *env, jobject thiz, jboolean input);
	JNIEXPORT void JNICALL JNI_API_NAME(queueInputBuffer)(JNIEnv *env, jobject thiz, jint index, jint offset, jint size, jlong timestampUs, jint flags);
	JNIEXPORT jint JNICALL JNI_API_NAME(dequeueInputBuffer)(JNIEnv *env, jobject thiz, jlong timeoutUs);
	JNIEXPORT jint JNICALL JNI_API_NAME(dequeueOutputBuffer)(JNIEnv *env, jobject thiz, jobject bufferInfo, jlong timeoutUs);
	JNIEXPORT void JNICALL JNI_API_NAME(releaseOutputBuffer)(JNIEnv *env, jobject thiz, jint index, jboolean render);
	JNIEXPORT void JNICALL JNI_API_NAME(startCodec)( JNIEnv *env, jobject thiz, jobject bufferInfo);
	JNIEXPORT void JNICALL JNI_API_NAME(native_init)(JNIEnv *env, jobject thiz);
	JNIEXPORT void JNICALL JNI_API_NAME(native_setup)(JNIEnv *env, jobject thiz, jstring name, jboolean nameIsType, jboolean encoder);


	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartVideoSend)(JNIEnv *env, jobject, jstring destip, jshort localSendPort, jshort remotePort);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopVideoSend)(JNIEnv *env, jobject);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartVideoRecv)(JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags, jshort localRecvPort);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopVideoRecv)(JNIEnv *env, jobject);

#ifdef __cplusplus
}
#endif


#endif