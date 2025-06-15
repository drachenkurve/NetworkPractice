#pragma once

#include "Platform.h"
#include "SocketAddress.h"

class FTcpSocket;

enum class EChatConnectionState : u8
{
	Connected,
	Disconnecting,
	Disconnected,
};

struct FChatConnection
{
public:
	//FChatConnection(const std::shared_ptr<FTcpSocket>& InSocket, std::string_view InUserName, const FSocketAddress& InAddress);

	bool IsConnected() const;

	EChatConnectionState GetState() const;

	bool TryDisconnect();
	void SetDisconnected();

public:
	std::shared_ptr<FTcpSocket> Socket;
	std::string UserName;

	FSocketAddress Address;

protected:
	mutable std::mutex StateMutex;
	EChatConnectionState State;
};
