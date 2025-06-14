#pragma once

#include "Platform.h"
#include "Socket.h"

class FSocketAddress;

class FUdpSocket : public TSocket<FUdpSocket>
{
	friend class FSocketUtility;

public:
	FUdpSocket(SOCKET InSocket);

	bool SendTo(const u8* Data, i32 Count, i32& ByteCount, const FSocketAddress& Address, i32 flags = 0);
	bool RecvFrom(u8* Data, i32 Count, i32& ByteCount, FSocketAddress& OutAddress, i32 flags = 0);

	// TODO: multicast
};
