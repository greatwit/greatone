

#include "QueueThread.h"

#define SLEEP_TIME 1000

QueueThread::QueueThread()
		:mRunning(false)
		,mHead(NULL)
		,mDataCount(0)
{
	pthread_mutex_init(&mMutex,NULL);
	mCond  = PTHREAD_COND_INITIALIZER;        /// < 唤醒机制

	mTag[32] = {0};
}

QueueThread::~QueueThread()
{
	clearQueue();

	//pthread_mutex_destroy(&mMutex);
	//pthread_cond_destroy(&mCond);
}

bool QueueThread::StartThread(char *tag)
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

bool QueueThread::StopThread()
{
	mRunning = false;
	
	//if(mHead)
	//{
	//	SAFE_FREE(mHead->buff);
	//	SAFE_FREE(mHead);
	//}
	
	return true;
}

void* QueueThread::process(void* context)
{
	if(NULL == context)
		return 0;

	QueueThread *pContext = (QueueThread*)context;
	
	while(pContext->mRunning)
	{
		pContext->consumer();
		usleep(5*SLEEP_TIME);
	}
	return 0;
}

bool QueueThread::AddBuffer(char*buff, int len)
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

			pthread_cond_signal(&mCond);
		}
		else
			return false;

		return true;
}

 void QueueThread::consumer()
 {
    struct Buffer * pCurMsg;
    struct Buffer * pTempMsg;

	AUDIOLOGW("consume data  ---------------------%s 1 count: %d\n", mTag, mDataCount);

    pthread_mutex_lock(&mMutex);

    if (NULL == mHead)
        pthread_cond_wait(&mCond, &mMutex);

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

 void QueueThread::clearQueue()
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

 void QueueThread::ThreadLoop(char*buff, int len)
 {

 }


