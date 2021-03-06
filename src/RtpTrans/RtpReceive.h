/*
 * RtpReceive.h
 *
 *  Created on: Dec 28, 2015
 *      Author: 
 */

#ifndef RtpReceive_H_
#define RtpReceive_H_

#include <utils/threads.h>

#include "RTPSessionEx.h"

#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"

#include "CommonThread.h"

#include "IReceiveCallback.h"


#define TAG "RtpReceive"
#include "ComDefine.h"

using namespace android;
using namespace jrtplib;

class RtpReceive : public CommonThread
{
	public:
		RtpReceive();
		virtual ~RtpReceive();
		bool	initSession(short localPort);
		bool	deinitSession();
		void	registerCallback(IReceiveCallback *base);
		bool	startThread();
		bool	stopThread();
		std::string& getSourceIp();
		bool    connect(std::string sDestIp, short destPort);
		bool	disConnect();
		bool	sendBuffer(void*buff, int dataLen, char*hdrextdata, int numhdrextwords, int64_t timeStamp);

	protected:
		virtual bool	threadLoop();

	private:
		bool					mRunning;
		short					mLocalPort;
		RTPSessionEx			mSession;
		RTPIPv4Address			mDestAddr;

		IReceiveCallback		*mStreamBase;
};

#endif /* RtpReceive_H_ */
