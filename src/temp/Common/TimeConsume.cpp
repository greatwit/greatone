
#include "TimeConsume.h"


TimeConsume*TimeConsume::mpInstance = new TimeConsume(); 

TimeConsume* TimeConsume::GetInstance()  
{  
	if(mpInstance == NULL)
		mpInstance = new TimeConsume();  
	return mpInstance;  
}

TimeConsume::TimeConsume()
{

}

TimeConsume::~TimeConsume()
{

}

void TimeConsume::TimeBegin()
{
	gettimeofday(&mBeginTime , NULL);  
}

void TimeConsume::TimeEnd()
{
	gettimeofday(&mEndTime , NULL);  
}

int TimeConsume::TimeCountMicroSecond(char*tag)
{
	int iTime;
	TimeEnd();
	iTime = (mEndTime.tv_sec - mBeginTime.tv_sec)*1000000 + mEndTime.tv_usec - mBeginTime.tv_usec;
	ALOGD("%s debug time consume microsecond:%d", tag, iTime);
	return iTime;
}

int	TimeConsume::TimeCountMilliSecond (char*tag)
{
	int iTime;
	TimeEnd();
	iTime = (mEndTime.tv_sec - mBeginTime.tv_sec)*1000000 + mEndTime.tv_usec - mBeginTime.tv_usec;
	iTime = iTime/1000;

	ALOGD("%s debug time consume millisecond:%d", tag, iTime);
	return iTime;
}
