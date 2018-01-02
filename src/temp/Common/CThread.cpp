

#include "CThread.h"

#define SLEEP_TIME 5*1000

CThread::CThread():
		mRunning(false)
		,mMasterPoint(NULL)
		,mCallback(NULL)
		,mHead(NULL)
		,mDataCount(0)
{
	pthread_mutex_init(&mMutex,NULL);
	mCond  = PTHREAD_COND_INITIALIZER;        /// < ���ѻ���
}


CThread::~CThread()
{
	//pthread_mutex_destroy(&mMutex);
	//pthread_cond_destroy(&mCond);
}

bool CThread::start()
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
	return true;
}

bool CThread::stop()
{
	mRunning = false;
	
	//if(mHead)
	//{
	//	SAFE_FREE(mHead->buff);
	//	SAFE_FREE(mHead);
	//}
	
	return true;
}

void* CThread::process(void* context)
{
	if(NULL == context)
		return 0;

	CThread *pContext = (CThread*)context;
	
	while(pContext->mRunning)
	{
		pContext->consumer();
		usleep(SLEEP_TIME);
	}
	return 0;
}

void CThread::setCallback(void*context, HANDLE_DATA callback)
{
	mMasterPoint = context;
	mCallback	 = callback;
}

bool CThread::addBuffer(char*buff, int len)
{
		if(NULL!=buff&&len>0)
		{
			struct msg * pMsgNode;
			pMsgNode = (struct msg *)malloc(sizeof(struct msg));
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

 /**
  * @brief ������
  * @param �޸�:
  *         - �����β�����ѣ���������������������Ѳ����������ˣ���
  */
 void CThread::consumer()
 {
     struct msg * pCurMsg;
     struct msg * pTempMsg;

    pthread_mutex_lock(&mMutex);

    if (NULL == mHead)
        pthread_cond_wait(&mCond, &mMutex);

    pCurMsg = mHead;
    if (pCurMsg->next) /// < ����һ�����
    {
        while (pCurMsg->next)
        {
            pTempMsg = pCurMsg; /// < ǰһ��
            pCurMsg = pCurMsg->next; /// < �������һ��
        }
		pTempMsg->next = NULL; /// < �����,��ע��mpp->next��
    }
    else /// < ֻ��ͷ�������
    {
		mHead = NULL; /// < ����ģ���ע��head��
    }
	mDataCount--;
	ALOGV("consume data  --------------------- %d\n", mDataCount);
    pthread_mutex_unlock(&mMutex);

	if( mCallback!=NULL && mMasterPoint!=NULL )
		mCallback(mMasterPoint, pCurMsg->buff, pCurMsg->num);
	
	SAFE_FREE(pCurMsg->buff);
	SAFE_FREE(pCurMsg);
 }
