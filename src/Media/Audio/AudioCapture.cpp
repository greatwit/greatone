//#define LOG_NDEBUG 0


#include <fcntl.h> 
#include <sys/mman.h> 
#include <utils/threads.h>
#include <signal.h>

#include "AudioCapture.h"


const int SLEEP_TIME = 5*1000;


AudioCapture::AudioCapture():
	mbCapturing(false),
	mbInited(false),
	mpPcm(NULL),
	mBuffer(NULL),
	mStreamBase(NULL)
{
	//ALOGV("mVideoWidth: %d, mVideoHeight: %d", mVideoWidth, mVideoHeight);
}

AudioCapture::~AudioCapture()
{
	AUDIOLOGW("AudioCapture, Destructor");
}

bool AudioCapture::Init(int audioCard, int audioDevice)
{
	if(mbInited)
		return false;

	mSelectCount = 0;

	if(OpenDevice(audioCard, audioDevice)>=0) //AUDIO_CARD, AUDIO_DEVICE
	{
		mbInited = true;
	}
	else
		return false;
	
	return true;
}

bool AudioCapture::Deint()
{
	if(!mbInited)
		return mbInited;

	mbInited = false;

	AUDIOLOGW("TAG 11:function %s,line:%d TalkbackFinish::Deint finished...1", __FUNCTION__, __LINE__);
	CloseDevice();
	AUDIOLOGW("TAG 11:function %s,line:%d TalkbackFinish::Deint finished...2", __FUNCTION__, __LINE__);

	if(mBuffer)
		SAFE_FREE(mBuffer);

	return true;
}

bool AudioCapture::Start()
{
	AUDIOLOGW("TAG 4:%s,line=:%d, AudioCapture::Start-----------1",__FUNCTION__,__LINE__);
	if(!mbInited || mbCapturing)
		return mbCapturing;


	mbCapturing = true;

	AUDIOLOGW("TAG 11:function %s,line:%d",__FUNCTION__,__LINE__);
	run("AudioCapture", PRIORITY_URGENT_DISPLAY);
	AUDIOLOGW("TAG 12:function %s,line:%d",__FUNCTION__,__LINE__);

	ALOGW("TAG 4:%s,line=:%d, AudioCapture::Start-----------2 ",__FUNCTION__,__LINE__);

	return true;
}

bool AudioCapture::Stop()
{
	if(!mbCapturing)
		return mbCapturing;

	AUDIOLOGW("TAG 4:%s,line=:%d, TalkbackFinish::StopThread-----------1 ",__FUNCTION__,__LINE__);
	mbCapturing = false;
	requestExit();
	AUDIOLOGW("TAG 4:%s,line=:%d, TalkbackFinish::StopThread-----------2 ",__FUNCTION__,__LINE__);

	return true;
}

bool AudioCapture::OpenDevice(int audioCard, int audioDevice)
{
	bool bResult = false;

	if(mbInited)
		return false;

	struct pcm_config config;
	memset(&config, 0, sizeof(config));

	config.channels				= AUDIO_CHANNELS;
	config.rate					= AUDIO_SAMPLE_RATE;
	config.period_size			= AUDIO_PERIOD_SIZE;
	config.period_count			= AUDIO_PERIOD_COUNT;

	switch (AUDIO_BITS) 
	{
		case 32:
			config.format = PCM_FORMAT_S32_LE;
			break;

		case 24:
			config.format = PCM_FORMAT_S24_LE;
			break;

		case 16:
			config.format = PCM_FORMAT_S16_LE;
			break;

		default:
			config.format = PCM_FORMAT_S16_LE;
    }
		
	config.start_threshold		= 0;
	config.stop_threshold		= 0;
	config.silence_threshold	= 0;
	
	mpPcm = pcm_open(audioCard, audioDevice, PCM_IN, &config);//
	if (!mpPcm || !pcm_is_ready(mpPcm))
	{
		ALOGE("ERROR: %s, line: %d Unable to open PCM device (%s)", __FUNCTION__, __LINE__, pcm_get_error(mpPcm));
		return bResult;
	}

	mSize = pcm_frames_to_bytes(mpPcm, pcm_get_buffer_size(mpPcm));
	mSize /= 4;

	if(NULL == mBuffer)
	{
		mBuffer = (char*)malloc(mSize);
		if (!mBuffer) 
		{
			ALOGE("ERROR: %s, line: %d Unable to allocate %u bytes", __FUNCTION__, __LINE__, mSize);

			pcm_close(mpPcm);
			free(mBuffer);
			mBuffer = NULL;

			return bResult;
		}
	}

	mbInited = true;

	return true;
}

void AudioCapture::CloseDevice()
{
	pcm_close(mpPcm);
}

void AudioCapture::registerCallback(IAudioCallback *base)
{
	mStreamBase = base;
}

int AudioCapture::WriteData(const void *data, unsigned int count)
{
	return pcm_write(mpPcm, data, count);
}

int	AudioCapture::GetCaptureFrame()
{
	int res = 0;
	if(!mbInited)
		return res;
	/*
	fd_set fds;		
	struct timeval tv;
	int fd = pcm_get_fd(mpPcm);
	FD_ZERO(&fds);
	FD_SET(fd, &fds);		
	tv.tv_sec  = 1;
	tv.tv_usec = 0;

	res = select(fd + 1, &fds, NULL, NULL, &tv);
	
	if (res == 0) 
	{
		AUDIOLOGW("TAG 4:%s,line=:%d, select timeout ",__FUNCTION__,__LINE__);
		if(mSelectCount>0)
		{
			mStreamBase->AudioSource( mCurFrameTimestamp, mBuffer, -1);
			mSelectCount = -10;
		}
		else
			mSelectCount++;

		return 0;
	}
	else if (res == -1) 
	{
		AUDIOLOGW("TAG 4:%s,line=:%d, select err ",__FUNCTION__,__LINE__);
		return -1;
	} 
	*/
	//struct timeval tv;
	//gettimeofday(&tv , NULL);
	
	AUDIOLOGW("TAG 4:%s,line=:%d, ---Audio---Capture::GetCaptureFrame-----------1 mSize:%d", __FUNCTION__, __LINE__, mSize);
	res = pcm_read(mpPcm, mBuffer, mSize);
	AUDIOLOGW("TAG 4:%s,line=:%d, ---Audio---Capture::GetCaptureFrame-----------2 res:%d", __FUNCTION__, __LINE__, res);

	if( NULL!=mStreamBase && res>=0 )
	{
		mCurFrameTimestamp = 0;//tv.tv_sec*1000000 + tv.tv_usec;
		mStreamBase->AudioSource( mCurFrameTimestamp, mBuffer, mSize);
	}
	else
	{
		AUDIOLOGW( "pcm_read or callback failed result=: %d \n", res);
		mStreamBase->AudioSource( 0, mBuffer, -1);
		
		return res;
	}

	return 1;
}

bool AudioCapture::IsRunning()
{
	return mbCapturing;
}

bool AudioCapture::threadLoop()
{
	int ret = -1;

	while(mbCapturing)
	{
		ret = GetCaptureFrame();
		if(ret <= 0)
		{
			usleep(SLEEP_TIME*10);
			continue;
		}
	
		if(false == mbCapturing)
			return false;

		usleep(60*1000);
	}

	return false;
}


