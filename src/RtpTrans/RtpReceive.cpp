/*
 * RtpReceive.cpp
 *
 *  Created on: Dec 28, 2015
 *      Author: 
 */

#include "RtpReceive.h"


RtpReceive::RtpReceive():
	mRunning(false),
	mInited(false),
	mStreamBase(NULL)
{
}

RtpReceive::~RtpReceive() 
{
	// TODO Auto-generated destructor stub
}

bool RtpReceive::initSession(short localPort)
{
	int iStatus;
	int packet_num = 0;
	mLocalPort = localPort;

	if(mInited)
		return false;

	ALOGV("TAG :function %s,line:%d init localport:%d ",__FUNCTION__,__LINE__, mLocalPort);

	RTPSessionParams sessionparams;
	RTPUDPv4TransmissionParams transparams;

	sessionparams.SetOwnTimestampUnit(1.0/8000.0);
	sessionparams.SetAcceptOwnPackets(true); 
	//sessionparams.SetUsePollThread(true);				//background thread to call virtual callbacks - set by default, but just to be sure 
	//sessionparams.SetMaximumPacketSize(MAX_PACKET_SIZE); 

	transparams.SetPortbase(mLocalPort);
	iStatus = mSession.Create(sessionparams, &transparams);

	if (iStatus >= 0)
	{
		mSession.SetDefaultPayloadType(0);
		mSession.SetDefaultMark(false);
		mSession.SetMaximumPacketSize(MAX_PACKET_SIZE);
		mSession.SetDefaultTimestampIncrement(30);
	}
	else
	{
		ALOGE("function: %s, line: %d, mSession.Create failed error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		return false;
	}

	mInited = true;


	return true;
}

bool RtpReceive::deinitSession()
{
	if(false == mInited)
		return mInited;

	//mSession.ClearDestinations();

	mSession.Destroy();

	mInited = false;

	return true;
}

void RtpReceive::registerCallback(IReceiveCallback *base)
{
	mStreamBase = base;
}

bool RtpReceive::startThread()
{
	if(false == mInited)
		return mInited;


	mRunning = true;
	ALOGD("TAG 11:function %s,line:%d",__FUNCTION__,__LINE__);
	run("RtpReceive", PRIORITY_URGENT_DISPLAY);
	ALOGD("TAG 12:function %s,line:%d",__FUNCTION__,__LINE__);

	return true;
}

bool RtpReceive::stopThread()
{
	if(false == mRunning)
		return false;

	mRunning = false;
	requestExit();

	return true;
}

string& RtpReceive::getSourceIp()
{
	return mSession.getSourceIp();
}

bool RtpReceive::connect(std::string sDestIp, short destPort)
{
	int iStatus=0;
	if(false == mInited)
		return mInited;

	uint32_t uiDestip = inet_addr(sDestIp.c_str());
	if (uiDestip == INADDR_NONE)
	{
		ALOGE("TAG 12:function %s,line:%d Bad IP address specified",__FUNCTION__,__LINE__);
		return false;
	}

	uiDestip = ntohl(uiDestip);

	mDestAddr.SetIP(uiDestip);
	mDestAddr.SetPort(destPort);

	iStatus = mSession.AddDestination(mDestAddr);
	if(iStatus<0)
	{
		ALOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		return false;
	}

	return true;
}


bool RtpReceive::disConnect()
{
	mSession.DeleteDestination(mDestAddr);
	ALOGW("TAG 12:function %s,line:%d RtpSender::disConnect 1",__FUNCTION__,__LINE__);
	mSession.ClearDestinations();
	ALOGW("TAG 12:function %s,line:%d RtpSender::disConnect 2",__FUNCTION__,__LINE__);
	return true;
}

bool RtpReceive::sendBuffer(void*buff, int dataLen, char*hdrextdata, int numhdrextwords, int64_t timeStamp)
{
	int iStatus=0;

	if(dataLen>=MAX_PACKET_SIZE)
	{
		ALOGE("function: %s, line: %d, data len big than max_packet_size.", __FUNCTION__, __LINE__);
		return false;
	}

	TESTLOGW("function: %s, line: %d, ========================3 len:%d", __FUNCTION__, __LINE__, dataLen);
	iStatus = mSession.SendPacketEx((void *)buff, dataLen, 0, hdrextdata, numhdrextwords); 
	TESTLOGW("function: %s, line: %d, ========================4 ", __FUNCTION__, __LINE__);

	if(iStatus<0)
	{
		ALOGE("function: %s, line: %d, error: %s", __FUNCTION__, __LINE__, RTPGetErrorString(iStatus).c_str());
		return false;
	}

	return true;
}

bool RtpReceive::threadLoop()
{
	int iStatus;
	while(mRunning)
	{
		mSession.BeginDataAccess();
		unsigned char *buff = NULL;
		
		if (mSession.GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket *pack;
				while( mRunning && (pack = mSession.GetNextPacket()) !=NULL)
				{
					if (pack->GetPayloadLength()>0)
					{
						buff = pack->GetPayloadData();
						
						if(mStreamBase!=NULL && mRunning)
						{
								mStreamBase->ReceiveSource(pack->GetTimestamp(), (char*)pack->GetExtensionData(), (void *)buff, pack->GetPayloadLength());
						}
						else
							return false;
					}
					
					ALOGD("got packet! GetExtensionData:%s payloadlength:%d id:%d count:%d source ip:%s \n",
							pack->GetExtensionData(), pack->GetPayloadLength(), pack->GetExtensionID(), pack->GetTimestamp(), mSession.getSourceIp().c_str());

					if(pack->HasMarker())
					{
					}
					mSession.DeletePacket(pack);
				}
				usleep(20000);
			}while(mRunning && mSession.GotoNextSourceWithData());
		}

		mSession.EndDataAccess();
	}

	RTPTime::Wait(RTPTime(0, 5*1000));
	
	return false;
}

