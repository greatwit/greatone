#ifndef CODEC_SENDER_H_
#define CODEC_SENDER_H_

#include <utils/threads.h> 
#include <utils/Log.h>


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

#include <camera/Camera.h>

#include "RtpSender.h"
#include "IVideoCallback.h"
#include "CodecContext.h"

#include "CameraContext.h"

using namespace android;

class CodecSender : public ICodecCallback, public IVideoCallback
{
	public:
		CodecSender();
		virtual ~CodecSender();
		
		bool CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort);
		bool CreateCodec(jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort, int cameraId);
		
		bool DeInit();
		
		void SetCameraParameter(jstring params);
		jstring GetCameraParameter();
		
		bool ConnectDest(std::string ip, short port);
		bool StartVideo(const sp<Surface> &cameraSurface);
		bool StopVideo();

		void AddDecodecSource(char *data, int len);
		virtual void onCodecBuffer(struct CodecBuffer& buff);
		virtual void VideoSource(V4L2BUF_t *pBuf);
		
		
	protected:

	private:
		bool				mFirstFrame;
		bool				mbRunning;
		//sp<Camera> 			mCamera;

		//sp<CodecBase> 	mCodec;
		RtpSender					*mpSender;
};

#endif


