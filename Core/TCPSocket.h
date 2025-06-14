#pragma once

#include "Platform.h"
#include "Socket.h"

class FSocketAddress;

class FTcpSocket : public TSocket<FTcpSocket>
{
	friend class FSocketUtility;

public:
	FTcpSocket(SOCKET InSocket);

	bool Connect(const FSocketAddress& Address);
	bool Listen(i32 MaxBacklog);
	std::shared_ptr<FTcpSocket> Accept(FSocketAddress& OutAddress);
	bool Send(const u8* Data, i32 BufferSize, i32& ByteCount, i32 flags = 0);
	bool Recv(u8* Data, i32 BufferSize, i32& ByteCount, i32 flags = 0);

	bool SetNoDelay(bool bNoDelay);
};
