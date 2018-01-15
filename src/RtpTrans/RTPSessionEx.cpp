#include "RTPSessionEx.h"


RTPSessionEx::RTPSessionEx()
		:RTPSession()
		,mSourceIp("")
{
}

RTPSessionEx::~RTPSessionEx()
{
}

bool RTPSessionEx::IsSameAddress(const RTPAddress *addr) const
{
	return false;
}

bool RTPSessionEx::IsFromSameHost(const RTPAddress *addr) const
{
	return false;
}

string& RTPSessionEx::getSourceIp()
{
	return mSourceIp;
}

void RTPSessionEx::OnNewSource(RTPSourceData *dat)
{
	if (dat->IsOwnSSRC())
		return;

	uint32_t ip;
	uint16_t port;

	if (dat->GetRTPDataAddress() != 0)
	{
		const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
		ip = addr->GetIP();
		port = addr->GetPort();
	}
	else if (dat->GetRTCPDataAddress() != 0)
	{
		const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
		ip = addr->GetIP();
		port = addr->GetPort()-1;
	}
	else
		return;

	RTPIPv4Address dest(ip,port);
	AddDestination(dest);

	struct in_addr inaddr;
	inaddr.s_addr = htonl(ip);
	mSourceIp = string(inet_ntoa(inaddr));

	GLOGW("%s,line:%d Adding destination address:%s port:%d",__FUNCTION__,__LINE__ , inet_ntoa(inaddr), port);
}

void RTPSessionEx::OnBYEPacket(RTPSourceData *dat)
{
	if (dat->IsOwnSSRC())
		return;

	uint32_t ip;
	uint16_t port;

	if (dat->GetRTPDataAddress() != 0)
	{
		const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
		ip = addr->GetIP();
		port = addr->GetPort();
	}
	else if (dat->GetRTCPDataAddress() != 0)
	{
		const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
		ip = addr->GetIP();
		port = addr->GetPort()-1;
	}
	else
		return;

	RTPIPv4Address dest(ip, port);
	DeleteDestination(dest);

	struct in_addr inaddr;
	inaddr.s_addr = htonl(ip);
	GLOGW("%s,line:%d Adding destination address:%s port:%d",__FUNCTION__,__LINE__ , inet_ntoa(inaddr), port);
}

void RTPSessionEx::OnRemoveSource(RTPSourceData *dat)
{
	if (dat->IsOwnSSRC())
		return;
	if (dat->ReceivedBYE())
		return;

	uint32_t ip;
	uint16_t port;

	if (dat->GetRTPDataAddress() != 0)
	{
		const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
		ip = addr->GetIP();
		port = addr->GetPort();
	}
	else if (dat->GetRTCPDataAddress() != 0)
	{
		const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
		ip = addr->GetIP();
		port = addr->GetPort()-1;
	}
	else
		return;

	RTPIPv4Address dest(ip, port);
	DeleteDestination(dest);

	struct in_addr inaddr;
	inaddr.s_addr = htonl(ip);
	GLOGW("%s,line:%d Adding destination address:%s port:%d",__FUNCTION__,__LINE__ , inet_ntoa(inaddr), port);
}
