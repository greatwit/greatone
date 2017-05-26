
#ifndef AUDIO_BASE_H
#define AUDIO_BASE_H

#include <string>
#include <utils/threads.h>

#include "RecordBase.h"

#include "QueueThread.h"

#include "ComDefine.h"
#define TAG "AudioBase"

#define G711_CODEC	false

class AudioBase : public QueueThread
{
	public:	
		AudioBase();
		virtual ~AudioBase();

		virtual bool Init(short port) = 0;
		virtual bool DeInit() = 0;
		virtual bool IsInited()	= 0;
		virtual bool Start(int cardid)	= 0;
		virtual bool Stop()	= 0;
		virtual void RegisterRecorder(RecordBase*record) = 0;

		virtual bool ConnectDest(std::string ip, short port);
		void  ThreadLoop(char*buff, int len);

		RecordBase *mpRecorder;
};


#endif //


