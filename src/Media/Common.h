#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>

#include <gui/Surface.h>
#include <media/ICrypto.h>
#include <media/stagefright/foundation/AMessage.h>
#include "android_runtime/AndroidRuntime.h"

#define ALOGTEST(...)		__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)			__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)
#define ALOGV(...)			__android_log_print(ANDROID_LOG_WARN,	TAG,  __VA_ARGS__)

using namespace android;


struct CodecBuffer
{
	char*buf;
	int size;
	uint32_t flags;
};

class ICodecCallback
{
	public:
		virtual ~ICodecCallback(){}
		virtual void onCodecBuffer(struct CodecBuffer& buff){}
};


typedef int test_t(int, int); // myadd function type

typedef status_t ConvertKeyValueToMessage_t(JNIEnv *env, jobjectArray keys, jobjectArray values, sp<AMessage> *msg);

typedef int   CodecCreate_t(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, bool isEncode);
typedef void  RegisterBufferCall_t(ICodecCallback *call);
typedef int   StartCodec_t();
typedef int   StopCodec_t();
typedef int   AddBuffer_t(char*data, int len);
typedef void  CodecDestroy_t();


namespace android 
{
	
struct AMessage;
struct ICrypto;

class CodecBaseLib
{
	public:
		CodecBaseLib();
		~CodecBaseLib();
		bool LoadBaseLib(int version);
		
		static CodecBaseLib*getInstance();
		
		CodecCreate_t*				CodecCreate;
		RegisterBufferCall_t*		RegisterBufferCall;
		StartCodec_t* 				StartCodec;
		StopCodec_t*				StopCodec;
		AddBuffer_t*				AddBuffer;
		CodecDestroy_t* 			CodecDestroy;
		ConvertKeyValueToMessage_t*	ConvertKeyValueToMessage;
		
		test_t* 		testAdd;
		
	private:
		static CodecBaseLib*mSelf;
		void *mLibHandle;
};

}


#endif


