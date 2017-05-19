#include "CardControl.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <android/log.h>

#define	 TAG "CardControl"
#define ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)



#define DEVICE "/dev/fm1188"

CardControl::CardControl()
			:mbInited(false)
			,mFd(-1)
{
}

CardControl::~CardControl()
{
	mbInited = false;
}

bool CardControl::Init()
{
	bool bResult = false;

	if(mbInited)
		return true;

	mFd = open(DEVICE, O_RDWR);
	if (mFd < 0)
	{
		ALOGE("function %s,line:%d pcm /dev/fm1188 open failed. fd:%d", __FUNCTION__, __LINE__, mFd);
		return bResult;
	}
	else
		ALOGE("function %s,line:%d pcm /dev/fm1188 open successfully. fd:%d", __FUNCTION__, __LINE__, mFd);

	mbInited = true;

	return true;
}

bool CardControl::DeInit()
{
	bool bResult = false;
	if(mbInited)
	{
		int res = close(mFd);
		ALOGE("function %s,line:%d pcm /dev/fm1188 close mfd res:%d.", __FUNCTION__, __LINE__, res);
	}
	
	mbInited = false;

	return true;
}

//设为内置声卡
bool CardControl::OpenInnerCard()
{
	bool bResult = false;
	
	if(!mbInited)
	{
		mFd = open(DEVICE, O_RDWR);
		if (mFd < 0)
		{
			ALOGE("function %s,line:%d pcm /dev/fm1188 open failed. fd:%d", __FUNCTION__, __LINE__, mFd);
			return bResult;
		}
		else
		{
			ALOGE("function %s,line:%d pcm /dev/fm1188 open successfully. fd:%d", __FUNCTION__, __LINE__, mFd);
			mbInited = true;
		}
	}
	
	if(mbInited)
	{
		int res = ioctl(mFd, 5, 1);
		ALOGE("function %s,line:%d pcm /dev/fm1188 OpenInnerCard. res:%d", __FUNCTION__, __LINE__, res);
		if(res<0)
		{
			return bResult;
		}
	}

	return true;
}

//设为外置声卡
bool CardControl::OpenOuterCard()
{
	bool bResult = false;
	
	if(!mbInited)
	{
		mFd = open(DEVICE, O_RDWR);
		if (mFd < 0)
		{
			ALOGE("function %s,line:%d pcm /dev/fm1188 open failed. fd:%d", __FUNCTION__, __LINE__, mFd);
			return bResult;
		}
		else
		{
			ALOGE("function %s,line:%d pcm /dev/fm1188 open successfully. fd:%d", __FUNCTION__, __LINE__, mFd);
			mbInited = true;
		}
	}
		
	if(mbInited)
	{
		int res = ioctl(mFd, 5, 0);
		ALOGE("function %s,line:%d pcm /dev/fm1188 OpenOuterCard. res:%d", __FUNCTION__, __LINE__, res);
		if(res<0)
		{
			return bResult;
		}
	}
	return true;
}

bool CardControl::SetSoundCardMode(bool talkback)
{
	bool bResult = false;
	if(talkback)
		bResult = OpenOuterCard();
	else
		bResult = OpenInnerCard();

	return bResult;
}

//up 4 down 3

//设为内置声卡
bool CardControl::VoiceUp()//媒体
{
	bool bResult = false;
	if(mbInited)
		if(ioctl(mFd, 4, 0)<0)
			return bResult;

	return true;
}

//设为外置声卡
bool CardControl::VoiceDown()//对讲
{
	bool bResult = false;
	if(mbInited)
		if(ioctl(mFd, 3, 0)<0)
			return bResult;

	return true;
}

bool CardControl::SetVolume(int value)//音量
{
	bool bResult = false;
	if(mbInited)
	{
		int res = ioctl(mFd, 7, value);
		ALOGE("function %s,line:%d pcm /dev/fm1188 SetVolume. res:%d", __FUNCTION__, __LINE__, res);
		if(res<0)
			return bResult;
	}

	return true;
}

bool CardControl::SetInnerVolume(int value)//音量
{
	bool bResult = false;
	if(OpenInnerCard())
	{
		bResult = SetVolume(value);
		//OpenInnerCard();
	}
	ALOGE("function %s,line:%d pcm /dev/fm1188 SetInnerVolume.", __FUNCTION__, __LINE__);

	return bResult;
}

bool CardControl::SetOuterVolume(int value)
{
	bool bResult = false;
	if(OpenOuterCard())
	{
		bResult = SetVolume(value);
		//OpenInnerCard();
	}
	ALOGE("function %s,line:%d pcm /dev/fm1188 SetOuterVolume.", __FUNCTION__, __LINE__);

	return bResult;
}


