#pragma once

#include "Platform.h"

struct FChatMessage;
class FTcpSocket;
class FIocp;
struct FChatConnection;
struct FIocpContext;

class FChatClient
{
public:
	FChatClient(HANDLE InStopEvent);
	virtual ~FChatClient();

	bool Startup(std::string_view IPv4String);
	void Cleanup();

	bool PostSend(const FChatMessage& Message);
	bool PostRecv();

	void HandleSend(FIocpContext& SendContext);
	void HandleRecv(FIocpContext& RecvContext);

protected:
	void RecvFunction();

protected:
	std::atomic<bool> bRunning;
	HANDLE StopEvent;

	std::unique_ptr<FIocp> Iocp;
	std::shared_ptr<FTcpSocket> Socket;

	std::thread RecvThread;

	mutable std::mutex SendMutex;
};
