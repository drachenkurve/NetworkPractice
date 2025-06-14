#pragma once

#include "Platform.h"
#include "SocketAddress.h"

class FTcpSocket;

struct FConnection
{
public:
	FConnection(const std::shared_ptr<FTcpSocket>& InSocket, std::string_view InUserName, const FSocketAddress& InAddress);

public:
	std::shared_ptr<FTcpSocket> Socket;
	std::string UserName;

	FSocketAddress Address;

	bool bConnected;
};
