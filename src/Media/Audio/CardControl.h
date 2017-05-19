#ifndef CARD_CONTROL_H
#define CARD_CONTROL_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>


class CardControl
{
	public:
		CardControl();
		~CardControl();
		bool Init();
		bool DeInit();
		bool OpenInnerCard();
		bool OpenOuterCard();

		bool VoiceUp();
		bool VoiceDown();
		bool SetVolume(int value);
		bool SetInnerVolume(int value);
		bool SetOuterVolume(int value);
		bool SetSoundCardMode(bool talkback);

	private:
		int mFd;
		bool mbInited;
};

#endif