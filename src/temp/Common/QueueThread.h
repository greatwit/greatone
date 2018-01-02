#ifndef QUEUE_THREAD_H_
#define QUEUE_THREAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include "ComDefine.h"




//线程基类，实现线程的创建及删除
class QueueThread
{
public:
    QueueThread();
    virtual ~QueueThread();
	bool  StartThread(char *tag);
	bool  StopThread();
	bool  AddBuffer(char*buff, int len);
	virtual void  ThreadLoop(char*buff, int len);
	
	

protected:
	static void* process(void* context);
	void  consumer();
	void  clearQueue();

private:
	struct Buffer*			mHead;
	pthread_t				mPid;

	pthread_mutex_t			mMutex;    /// < 互斥锁
	pthread_cond_t			mCond;        /// < 唤醒机制

	int						mDataCount;
	bool					mRunning;
	char					mTag[32];
};

#endif 


