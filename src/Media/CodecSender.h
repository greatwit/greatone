#ifndef CODEC_SENDER_H_
#define CODEC_SENDER_H_

#include <utils/threads.h> 
#include <utils/Log.h>

#include "RtpSender.h"
#include <string>
#include <map>

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

#include "CameraHardware.h" 
#include "IVideoCallback.h"

#include <gui/Surface.h> 

using namespace android;

class CodecSender : public ICodecCallback
{
	public:
		CodecSender();
		virtual ~CodecSender();
		bool CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags);
		bool CreateCodec(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort);
		
		bool DeInit();
		
		bool ConnectDest(std::string ip, short port);
		bool StartVideo(int deivceid);
		bool StopVideo();

		void AddDecodecSource(char *data, int len);
		void onCodecBuffer(struct CodecBuffer& buff);
		
	protected:


	private:
		bool				mFirstFrame;
		bool				mbRunning;
		
		sp<CodecBase> 	mCodec;
		RtpSender					*mpSender;
};

#endif


