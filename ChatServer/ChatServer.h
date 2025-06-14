#pragma once

#include "Platform.h"
#include "SocketAddress.h"

struct FConnection;
struct FIocpContext;
class FTcpSocket;
class FIocp;

class FChatServer
{
public:
	FChatServer();
	virtual ~FChatServer();

	bool Startup(u16 Port, i32 ThreadCount, i32 AcceptCountFallback = 16);
	void Cleanup();

	bool PostAccept();

	// ByteCount will be set if immediately processed, 0 if pending, unmodified if error
	bool PostSend(FIocpContext& SendContext, u32& ByteCount, u32 Flags = 0);

	// ByteCount will be set if immediately processed, 0 if pending, unmodified if error
	bool PostRecv(FIocpContext& RecvContext, u32& ByteCount, u32 Flags = 0);

	void HandleAccept(FIocpContext& AcceptContext);
	void HandleSend(FIocpContext& SendContext);
	void HandleRecv(FIocpContext& RecvContext);

	void Disconnect(std::shared_ptr<FConnection> Connection);

protected:
	void WorkerFunction();

protected:
	bool bRunning;

	std::unique_ptr<FIocp> Iocp;
	std::shared_ptr<FTcpSocket> ListenSocket;

	FSocketAddress Address;

	std::vector<std::thread> WorkerThreads;

	std::mutex ConnectionMapMutex;
	std::unordered_map<SOCKET, std::shared_ptr<FConnection>> ConnectionMap;
};
