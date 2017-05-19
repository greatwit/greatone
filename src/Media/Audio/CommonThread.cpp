

#include "CommonThread.h"

#define SLEEP_TIME 1000

CommonThread::CommonThread()
{
}

CommonThread::~CommonThread()
{
}

bool CommonThread::StartThread()
{
	int iResult;
	iResult = pthread_create(&mPid, NULL, ThreadWork, this);
	if (0 != iResult)
	{
		ALOGV("create thread1 failed:----!%s\n", strerror(iResult));
		return false;
	}
	return true;
}

bool CommonThread::StopThread()
{
	pthread_join(mPid, NULL);

	return true;
}

bool CommonThread::run(char*tag, int level)
{
	int iResult;
	iResult = pthread_create(&mPid, NULL, ThreadWork, this);
	if (0 != iResult)
	{
		ALOGV("create thread1 failed:----!%s\n", strerror(iResult));
		return false;
	}
	return true;
}

bool CommonThread::requestExit()
{
	pthread_join(mPid, NULL);

	return true;
}

void* CommonThread::ThreadWork(void* context)
{
	if(NULL == context)
		return 0;

	CommonThread *pContext = (CommonThread*)context;
	
	pContext->threadLoop();

	return 0;
}

 bool CommonThread::threadLoop()
 {
	 return true;
 }


