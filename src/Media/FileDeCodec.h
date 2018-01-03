#ifndef FILE_DE_CODEC_H
#define FILE_DE_CODEC_H

#include <utils/threads.h> 
#include <utils/Log.h>

#include "RtpSender.h"
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


#include "IVideoCallback.h"
#include "CodecContext.h"


using namespace android;

class FileDeCodec : public Thread, public ICodecCallback
{
	public:
		FileDeCodec();
		virtual ~FileDeCodec();

		//bool CreateCodec(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, char*filename);
		bool CreateCodec(const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, char*filename);
		bool DeInit();
		bool StartVideo(int deivceid);
		bool StopVideo();

		
	protected:
		bool isFirstFrame();
		void setFirstFrame(bool bFirs);

		void decorder(char*data, int dataLen);
		virtual bool threadLoop();

		void onCodecBuffer(struct CodecBuffer& buff);

	private:
		bool				mFirstFrame;
		bool				mbRunning;

		FILE 				*mFile;
		
		char mcharLength[4];
		char mData[1000000];
		
		//sp<CodecBase> 	mCodec;
};

#endif


