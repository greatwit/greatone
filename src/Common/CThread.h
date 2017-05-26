#ifndef THREAD_H_HEADER_INCLUDED_B223B067
#define THREAD_H_HEADER_INCLUDED_B223B067

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include "ComDefine.h"

#ifndef ALOGV
#define ALOGV printf
#endif

typedef void (* HANDLE_DATA)(void* pContext, char* buffer, int dataLen);

//消息头
struct msg
{
	int num;
	char*buff;
	struct msg * next;
};


//线程基类，实现线程的创建及删除
class CThread
{
public:
    CThread();
    virtual ~CThread();
	void  consumer();
	bool  start();
	bool  stop();
	void  setCallback(void*context, HANDLE_DATA callback);
	bool  addBuffer(char*buff, int len);
	bool mRunning;

protected:
	static void* process(void* context);

private:
	struct msg * mHead;
	pthread_t	mPid;

	pthread_mutex_t mMutex;    /// < 互斥锁
	pthread_cond_t	mCond;        /// < 唤醒机制

	void*                   mMasterPoint;
	HANDLE_DATA				mCallback;
	int						mDataCount;
	
};



#endif /* THREAD_H_HEADER_INCLUDED_B223B067 */
