#pragma once

#include "Platform.h"

template <typename T>
class TSocket
{
protected:
	TSocket(SOCKET InSocket);

	TSocket(const TSocket& other) = delete;
	TSocket(TSocket&& other) noexcept = delete;
	
	TSocket& operator=(const TSocket& other) = delete;
	TSocket& operator=(TSocket&& other) noexcept = delete;

	~TSocket();

public:
	bool IsValid() const;

	// effective when TCP socket
	bool Shutdown(i32 Mode);
	bool Close();

	bool SetNonBlocking(bool bNonBlocking);
	bool SetBroadcast(bool bBroadcast);
	bool SetSendBufferSize(i32 Size, i32& NewSize);
	bool SetRecvBufferSize(i32 Size, i32& NewSize);

protected:
	SOCKET Socket;
};

#include "Socket.inl"
