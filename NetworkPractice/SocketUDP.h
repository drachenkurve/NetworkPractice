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
	bool SendTo(const uint8_t* Data, int32_t Count, int32_t& ByteCount, const FSocketAddress& ToAddress);
	bool RecvFrom(uint8_t* Data, int32_t Count, int32_t& ByteCount, FSocketAddress& FromAddress);

private:
	SOCKET Socket;
};
