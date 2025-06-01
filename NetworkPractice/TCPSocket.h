#pragma once

#include "Platform.h"
#include "Socket.h"

class FSocketAddress;

class FTCPSocket : public TSocket<FTCPSocket>
{
	friend class FSocketUtility;

public:
	FTCPSocket(SOCKET InSocket);

	bool Connect(const FSocketAddress& Address);
	bool Listen(i32 MaxBacklog);
	std::shared_ptr<FTCPSocket> Accept(FSocketAddress& OutAddress);
	bool Send(const u8* Data, i32 BufferSize, i32& ByteCount, i32 flags = 0);
	bool Recv(u8* Data, i32 BufferSize, i32& ByteCount, i32 flags = 0);

	bool SetNoDelay(bool bNoDelay);
};
