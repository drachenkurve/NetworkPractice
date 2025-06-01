#pragma once

#include "Platform.h"

class FSocketAddress
{
	friend class FSocketUtility;

public:
	FSocketAddress(const sockaddr& InSockAddr);

	void Clear();

public:
	sockaddr SockAddr;
};
