#ifndef TEST_FILE_CODEC_H
#define TEST_FILE_CODEC_H

#include <utils/threads.h> 
#include <utils/Log.h>

#include "RtpSender.h"
#include <string>
#include <map>


#include "IVideoCallback.h"
#include "IReceiveCallback.h"

#include "CodecBase.h"

#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include <media/ICrypto.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>

#include <gui/Surface.h>

using namespace android;

class TestFileCodec : public Thread, public ICodecCallback
{
	public:
		TestFileCodec();
		virtual ~TestFileCodec();
		bool CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags);
		bool CreateCodec(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags);
		
		bool DeInit();
		
		bool StartVideo(int deivceid);
		bool StopVideo();

		void onCodecBuffer(struct CodecBuffer& buff);
		
		
	protected:
		bool isFirstFrame();
		void setFirstFrame(bool bFirs);

		void decorder(char*data, int dataLen);
		virtual bool threadLoop();


	private:
		bool				mFirstFrame;
		bool				mbRunning;

		FILE 				*mFile;
		
		char mcharLength[4];
		char mData[1000000];
		
		sp<CodecBase> 	mCodec;
};

#endif


