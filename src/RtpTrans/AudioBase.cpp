
#include "AudioBase.h"



AudioBase::AudioBase()
		:QueueThread()
		,mpRecorder(NULL)
{

}

AudioBase::~AudioBase()
{

}

bool AudioBase::ConnectDest(std::string ip, short port)
{
	return true;
}

void AudioBase::ThreadLoop(char*buff, int len)
{

}


