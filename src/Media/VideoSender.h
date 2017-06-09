#ifndef VIDEO_SENDER_H
#define VIDEO_SENDER_H

#include <utils/threads.h> 
#include <utils/Log.h>

#include "RtpSender.h"
#include <string>
#include <map>


#include "VideoBase.h"
#include "IVideoCallback.h"

#include "IReceiveCallback.h"

using namespace android;

class VideoSender : public VideoBase, public IVideoCallback
{
	public:
		VideoSender();
		virtual ~VideoSender();
		bool Init(ANativeWindow* window, int widht, int height, short sendPort);
		bool DeInit();
		bool IsInited();
		bool StartVideo(int deivceid);
		bool StopVideo();
		void RegisterRecorder(RecordBase*record);


		bool ConnectDest(std::string ip, short port);
		bool ChangePixel(int width, int height);

			
		bool IsRunning();
		bool sendBuffer(void*buff, int dataLen, char*hdrextdata, int numhdrextwords, int64_t timeStamp);

		void	registerCallback(IReceiveCallback *base);
		
	protected:
		bool isFirstFrame();
		void setFirstFrame(bool bFirs);

			
		virtual bool threadLoop();


	private:

		int					mVideoWidth;
		int					mVideoHeight;
		bool				mFirstFrame;
		bool				mbFirstWrite;
		bool				mbInited;
		bool				mbRunning;

		RtpSender					*mpSender;
		FILE 				*mFile;
		
		char mcharLength[4];
		char mData[1000000];
		
		IReceiveCallback		*mStreamBase;
};

#endif


