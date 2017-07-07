#ifndef CODEC_MEDIA_HEAD_H_
#define CODEC_MEDIA_HEAD_H_



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
	
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartVideoRecv)(JNIEnv *env, jobject, jshort localRecvPort);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopVideoRecv)(JNIEnv *env, jobject);

	
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StartFileDecoder)(JNIEnv *env, jobject, 
											jobjectArray keys, jobjectArray values,
											jobject jsurface,
											jobject jcrypto,
											jint flags);
	JNIEXPORT jboolean JNICALL JNI_API_NAME(StopFileDecoder)(JNIEnv *env, jobject);
#ifdef __cplusplus
}
#endif


#endif