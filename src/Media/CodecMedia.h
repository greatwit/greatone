#ifndef CODEC_MEDIA_HEAD_H_
#define CODEC_MEDIA_HEAD_H_



#define ALOGTEST(...)		__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)

#define JNI_API_NAME(A)		Java_com_great_happyness_Codec_CodecMedia_##A 


#ifdef __cplusplus
extern "C"
{
#endif

	JNIEXPORT void JNICALL JNI_API_NAME(native_configure)( JNIEnv *env, jobject thiz, jobjectArray keys, jobjectArray values, jobject jsurface, jobject jcrypto, jint flags);
	JNIEXPORT void JNICALL JNI_API_NAME(startCodec)( JNIEnv *env, jobject thiz);


	
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartVideoSend)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags,
											jobject bufferInfo);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopVideoSend)(JNIEnv *env, jobject);
	

	
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartFileDecoder)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopFileDecoder)(JNIEnv *env, jobject);


	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartFileEncoder)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopFileEncoder)(JNIEnv *env, jobject);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(AddEncoderData)(JNIEnv *env, jobject, jbyteArray byteData, jint len);


	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartCodecSender)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface, jobject jcrypto, jstring destip, 
											jshort destport, jshort localport, jint flags);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(CodecSenderData)(JNIEnv *env, jobject, jbyteArray byteData, jint len);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopCodecSender)(JNIEnv *env, jobject);


	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartCodecRecver)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface, jobject jcrypto, jint flags,
											jshort recvport);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopCodecRecver)(JNIEnv *env, jobject);
#ifdef __cplusplus
}
#endif


#endif
