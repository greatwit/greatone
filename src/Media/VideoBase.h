
#ifndef VIDEO_BASE_H
#define VIDEO_BASE_H

#include <string>
#include <utils/threads.h>
#include <android/native_window.h>

#include "RecordBase.h"

#include "ComDefine.h"
#define TAG "VideoBase"

#define H264_SPS_PPS_LEN			21

using namespace android;

class VideoBase : public Thread
{
	public:	
		VideoBase();
		virtual ~VideoBase();

		virtual bool Init(ANativeWindow *pNativeWindow, int nWidth, int nHeight, short port);
		virtual bool DeInit() = 0;
		virtual bool StartVideo(int cardid)	= 0;
		virtual bool StopVideo()	= 0;
		virtual void RegisterRecorder(RecordBase*record) = 0;

		virtual bool ConnectDest(std::string ip, short port);
		virtual bool threadLoop();

		RecordBase	*mpRecorder;
		bool		mbFirstFrame;
		char		mCharSpsPps[H264_SPS_PPS_LEN+1];
};


#endif //

