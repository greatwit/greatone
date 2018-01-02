

#include "SynQueueThread.h"

#define SLEEP_TIME 1000

SynQueueThread::SynQueueThread()
		:mRunning(false)
		,mHead(NULL)
		,mDataCount(0)
{
	pthread_mutex_init(&mMutex,NULL);

	mTag[32] = {0};
}

SynQueueThread::~SynQueueThread()
{
	usleep(120000); //
	clearQueue();

	pthread_mutex_destroy(&mMutex);
	//pthread_cond_destroy(&mCond);
}

bool SynQueueThread::StartThread(char *tag)
{
	int iResult;
	mRunning = true;
	iResult = pthread_create(&mPid, NULL, process, this);
	if (0 != iResult)
	{
		mRunning = false;
		ALOGV("create thread1 failed:----!%s\n", strerror(iResult));
		return false;
	}

	if(tag)
	{
		strcpy(mTag, tag);
	}

	return true;
}

bool SynQueueThread::StopThread()
{
	mRunning = false;
	
	//if(mHead)
	//{
	//	SAFE_FREE(mHead->buff);
	//	SAFE_FREE(mHead);
	//}
	
	return true;
}

bool SynQueueThread::run(char*tag, int level)
{
	int iResult;

	mRunning = true;
	
	pthread_attr_t attr;
	iResult = pthread_attr_init(&attr);

	iResult = pthread_attr_setschedpolicy (&attr, SCHED_RR );//SCHED_RR SCHED_FIFO SCHED_OTHER
	if (0 != iResult)
	{
		ALOGE("pthread_attr_setschedpolicy failed:----!%s\n", strerror(iResult));
	}
	iResult = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (0 != iResult)
	{
		ALOGE("pthread_attr_setdetachstate failed:----!%s\n", strerror(iResult));
	}
	//pthread_attr_setschedparam ( &attr, &sched );

	iResult = pthread_create(&mPid, &attr, process, this);
	pthread_attr_destroy (&attr);

	if (0 != iResult)
	{
		ALOGE("create thread1 failed:----!%s\n", strerror(iResult));
		return false;
	}
	
	pthread_detach(mPid);
	
	return true;
}

bool SynQueueThread::requestExit()
{
	mRunning = false;

	return true;
}

void* SynQueueThread::process(void* context)
{
	if(NULL == context)
		return 0;

	SynQueueThread *pContext = (SynQueueThread*)context;
	
	while(pContext->mRunning)
	{
		pContext->consumer();
		//usleep(5*SLEEP_TIME);
	}
	return 0;
}

bool SynQueueThread::AddBuffer(char*buff, int len)
{
		if(NULL!=buff && len>0)
		{
			struct Buffer * pMsgNode;
			pMsgNode = (struct Buffer *)malloc(sizeof(struct Buffer));
			pMsgNode->buff = (char *)malloc(len);
			pMsgNode->num = len;
			memcpy(pMsgNode->buff, buff, len);

			pthread_mutex_lock(&mMutex);
			pMsgNode->next = mHead;
			mHead = pMsgNode;
			mDataCount++;
			pthread_mutex_unlock(&mMutex);
		}
		else
			return false;

		return true;
}

 void SynQueueThread::consumer()
 {
    struct Buffer * pCurMsg;
    struct Buffer * pTempMsg;

	AUDIOLOGW("consume data  ---------------------%s 1 count: %d\n", mTag, mDataCount);

    pthread_mutex_lock(&mMutex);

    if (NULL == mHead)
	{
		pthread_mutex_unlock(&mMutex);

		ThreadLoop("", 0);
		return;
	}

    pCurMsg = mHead;
    if (pCurMsg->next) /// < 超过一个结点
    {
        while (pCurMsg->next)
        {
            pTempMsg = pCurMsg; /// < 前一个
            pCurMsg = pCurMsg->next; /// < 链表最后一个
        }
		pTempMsg->next = NULL; // < 必须的,且注意mpp->next！
    }
    else /// < 只有头结点的情况
    {
		mHead = NULL; /// < 必须的，且注意head！
    }
	mDataCount--;
	
    pthread_mutex_unlock(&mMutex);

	ThreadLoop(pCurMsg->buff, pCurMsg->num);

	SAFE_FREE(pCurMsg->buff);
	SAFE_FREE(pCurMsg);

	AUDIOLOGW("consume data  ---------------------%s 2 count: %d\n", mTag, mDataCount);
 }

 void SynQueueThread::clearQueue()
 {
	 struct Buffer * pCurMsg;
	 struct Buffer * pTempMsg;

	 pthread_mutex_lock(&mMutex);

	 if(mHead)
	 {
		 pCurMsg = mHead;
		 if (pCurMsg->next) /// < 超过一个结点
		 {
			 while (pCurMsg->next)
			 {
				 pTempMsg = pCurMsg; /// < 前一个
				 pCurMsg = pCurMsg->next; /// < 链表最后一个

				 SAFE_FREE(pTempMsg->buff);
				 SAFE_FREE(pTempMsg);
			 }
		 }
		 SAFE_FREE(pCurMsg->buff);
		 SAFE_FREE(pCurMsg);
	 }

	 pthread_mutex_unlock(&mMutex);
 }

 void SynQueueThread::ThreadLoop(char*buff, int len)
 {

 }


