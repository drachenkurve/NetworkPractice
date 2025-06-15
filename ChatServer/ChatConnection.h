#pragma once

#include "Platform.h"
#include "SocketAddress.h"

class FTcpSocket;

struct FChatConnection
{
public:
	FChatConnection(const std::shared_ptr<FTcpSocket>& InSocket, std::string_view InUserName, const FSocketAddress& InAddress);

public:
	std::shared_ptr<FTcpSocket> Socket;
	std::string UserName;

	FSocketAddress Address;

	std::atomic<bool> bConnected;
};
