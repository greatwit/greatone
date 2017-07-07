

#ifndef VIDEO_RECEIVE_H_
#define VIDEO_RECEIVE_H_

#include <utils/threads.h>

#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <string>


#include "CodecBase.h"

#include "RtpReceive.h"
#include "VideoBase.h"


using namespace android;


class VideoReceiver : public VideoBase, public IReceiveCallback
{
	public:
		VideoReceiver();
		virtual ~VideoReceiver();

		bool Init(sp<AMessage> &format, sp<Surface> &surface, sp<ICrypto> &crypto, int flags, short recvPort);
		bool DeInit();
		bool IsInited();
		bool StartVideo(int cardid);
		bool StopVideo();
		void RegisterRecorder(RecordBase*record);

		void ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen);

	protected:
		bool threadLoop();
		bool setBuffer(void* data, int len, int64_t timeStamp);
		bool isFirstFrame();
		void setFirstFrame(bool bFirst);

	private:
		bool mbInited;

	private:
	    sp<CodecBase>  			mCodec;
		RtpReceive					*mpReceive;
		
		bool						mbFirstWrite;
		bool						mRunning;
		bool 						mbTakeSnapShot;
	
		
};


#endif



