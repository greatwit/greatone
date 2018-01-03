#ifndef CODEC_RECEIVER_H
#define CODEC_RECEIVER_H

#include <utils/threads.h> 
#include <utils/Log.h>

#include "RtpReceive.h"
#include <string>
#include <map>






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


#include "IReceiveCallback.h"
#include "CodecContext.h"


using namespace android;

class CodecReceiver : public IReceiveCallback
{
	public:
		CodecReceiver();
		virtual ~CodecReceiver();
		bool CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags);
		bool CreateCodec(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short recvPort);
		
		bool DeInit();
		
		bool StartVideo(int deivceid);
		bool StopVideo();

		void ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen);
		
	protected:
		bool isFirstFrame();
		void setFirstFrame(bool bFirs);

		void decorder(char*data, int dataLen);


	private:
		bool				mFirstFrame;
		bool				mbRunning;

		
		//sp<CodecBase> 			mCodec;
		RtpReceive			*mpReceive;
};

#endif


