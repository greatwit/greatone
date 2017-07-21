#ifndef RTP_SESSION_EXTERNT
#define RTP_SESSION_EXTERNT

#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsourcedata.h"
#include <iostream>
#include <string>
#include "ComDefine.h"

using namespace std;
using namespace jrtplib;

class RTPSessionEx : public RTPSession
{
	public:
		RTPSessionEx();
		~RTPSessionEx();
		string& getSourceIp();
		bool IsSameAddress(const RTPAddress *addr) const;
		bool IsFromSameHost(const RTPAddress *addr) const;

	protected:
		void OnNewSource(RTPSourceData *dat);
		void OnBYEPacket(RTPSourceData *dat);
		void OnRemoveSource(RTPSourceData *dat);

	private:
		string	mSourceIp;
};

#endif
