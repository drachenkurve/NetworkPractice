#pragma once

#include "Platform.h"

class FSocketAddress;

class FSocketUDP
{
	friend class FSocketUtility;

public:
	FSocketUDP(SOCKET InSocket);
	~FSocketUDP();

	bool Bind(const FSocketAddress& Address);
	bool SendTo(const u8* Data, i32 Count, i32& ByteCount, const FSocketAddress& Address);
	bool RecvFrom(u8* Data, i32 Count, i32& ByteCount, FSocketAddress& OutAddress);

private:
	SOCKET Socket;
};
