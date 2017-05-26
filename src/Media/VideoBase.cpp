
#include "VideoBase.h"

VideoBase::VideoBase()
	:mpRecorder(NULL)
	,mbFirstFrame(true)
{
	memset(mCharSpsPps, 0 ,sizeof(mCharSpsPps));
}

VideoBase::~VideoBase()
{

}

bool VideoBase::ConnectDest(std::string ip, short port)
{
	return true;
}

bool VideoBase::threadLoop()
{
	return true;
}

