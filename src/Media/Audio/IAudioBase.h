

#ifndef I_AUDIO_BASE
#define I_AUDIO_BASE


class IAudioBase
{
	public:
		virtual ~IAudioBase(){}
		virtual void AudioSource(int64_t timeStamp, void* buffer, int dataLen)=0;
};
#endif
