

#ifndef VIDEO_HARD_CODEC_H_
#define VIDEO_HARD_CODEC_H_


#include "android_media_MediaCodec.h"
#include "android_media_MediaCrypto.h"

using namespace android;

class VideoHardCodec
{
    public:
	VideoHardCodec(JNIEnv *env, jobject thiz);
	~VideoHardCodec();

	    void Create(jstring name,         
		jobjectArray keys, jobjectArray values,
		jobject jsurface,
		jobject jcrypto,
		jint flags);

	    void Finished();
	    void Start(jobject bufferInfo);
	    void Stop();
	    
	    void AddBuffer(char*data, int dataLen);

	private:
		sp<JMediaCodec> mCodec;
		JNIEnv *mEnv;
   		jobject mThiz;

		uint8_t*mBufferPoint[2];
		jobject mBufferInfo;
};

#endif
