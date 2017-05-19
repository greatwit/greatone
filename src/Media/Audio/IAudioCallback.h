

#ifndef I_AUDIO_CALLBACK
#define I_AUDIO_CALLBACK


class IAudioCallback
{
	public:
		virtual ~IAudioCallback(){}
		virtual void AudioSource(int64_t timeStamp, void* buffer, int dataLen)=0;
};
#endif
