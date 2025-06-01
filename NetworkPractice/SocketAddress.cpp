#include "SocketAddress.h"

FSocketAddress::FSocketAddress()
{
	Clear();
}

FSocketAddress::FSocketAddress(const sockaddr& InSockAddr)
{
	memcpy(&SockAddr, &InSockAddr, sizeof(sockaddr));
}

void FSocketAddress::Clear()
{
	memset(&SockAddr, 0, sizeof(SockAddr));
}
