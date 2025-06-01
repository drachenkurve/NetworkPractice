#include "SocketAddress.h"

FSocketAddress::FSocketAddress(const sockaddr& InSockAddr)
{
	memcpy(&SockAddr, &InSockAddr, sizeof(sockaddr));
}

void FSocketAddress::Clear()
{
	memset(&SockAddr, 0, sizeof(SockAddr));
	SockAddr.sa_family = AF_UNSPEC;
}
