#pragma once

#include "Platform.h"

class FSocketAddress;

class FSocketTCP
{
	friend class FSocketUtility;

public:
	FSocketTCP(SOCKET InSocket);
	~FSocketTCP();

	bool Connect(const FSocketAddress& Address);
	bool Listen(i32 MaxBacklog);
	std::shared_ptr<FSocketTCP> Accept(FSocketAddress& OutAddress);
	bool Send(const u8* Data, i32 BufferSize, i32& ByteCount);
	bool Recv(u8* Data, i32 BufferSize, i32& ByteCount);

private:
	SOCKET Socket;
};
