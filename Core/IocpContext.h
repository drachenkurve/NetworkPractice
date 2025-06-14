#pragma once

#include "Platform.h"
#include "SocketAddress.h"

class FTcpSocket;

enum class EIocpOperationType : u8
{
	Accept,
	Send,
	Recv,
};

struct FIocpContext
{
public:
	FIocpContext(EIocpOperationType InOperationType, const std::shared_ptr<FTcpSocket>& InSocket);

public:
	OVERLAPPED Overlapped;
	EIocpOperationType OperationType;

	FSocketAddress Address;

	std::vector<u8> Buffer;
	std::shared_ptr<FTcpSocket> Socket;
};
