#ifndef AUDIO_HARDWARE_H
#define AUDIO_HARDWARE_H

#include <linux/videodev.h> 
#include <utils/threads.h>
#include <utils/Log.h>
#include "asoundlib.h"
#include "IAudioCallback.h"

#include "CommonThread.h"


#include "ComDefine.h"
#define TAG "AudioCapture"


using namespace android;

class AudioCapture: public CommonThread
{
	public:	
		AudioCapture();
		~AudioCapture();
		bool			Init(int audioCard, int audioDevice);
		bool			Deint();
		bool			Start();
		bool			Stop();
		void			registerCallback(IAudioCallback *base);
		bool			IsRunning();
		int             WriteData(const void *data, unsigned int count);

	protected:
		bool 			OpenDevice(int audioCard, int audioDevice);
		void 			CloseDevice();
		int				GetCaptureFrame();
		virtual bool	threadLoop();

	private:
		unsigned int			mSize;
		int						mSelectCount;
		bool					mbInited;
		bool					mbCapturing;
		struct pcm				*mpPcm;
		char					*mBuffer;

		int64_t 				mCurFrameTimestamp;
		IAudioCallback			*mStreamBase;

};


#endif // CAMERA_HARDWARE_H
