#include "ChatServer.h"

#include <ranges>

#include "Connection.h"
#include "Iocp.h"
#include "IocpContext.h"
#include "PlatformWindowsUtility.h"
#include "TcpSocket.h"

LPFN_ACCEPTEX AcceptExFunc = nullptr;
LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrsFunc = nullptr;

FChatServer::FChatServer():
	bRunning{false}
{
}

FChatServer::~FChatServer()
{
}

bool FChatServer::Startup(u16 Port, i32 ThreadCount, i32 AcceptCountFallback)
{
	Iocp = std::make_unique<FIocp>(ThreadCount);
	if (!Iocp || !Iocp->IsValid())
	{
		return false;
	}

	ListenSocket = FSocketUtility::CreateIPv4TCP();
	if (!ListenSocket || !ListenSocket->IsValid())
	{
		return false;
	}

	if (!AcceptExFunc)
	{
		GUID AcceptExGuid = WSAID_ACCEPTEX;
		DWORD ByteCount = 0;

		if (WSAIoctl(ListenSocket->GetNative(), SIO_GET_EXTENSION_FUNCTION_POINTER,
			&AcceptExGuid, sizeof(AcceptExGuid),
			&AcceptExFunc, sizeof(AcceptExFunc),
			&ByteCount, nullptr, nullptr) == SOCKET_ERROR)
		{
			FSocketUtility::ReportLastErrorCode();
			return false;
		}
	}

	if (!GetAcceptExSockaddrsFunc)
	{
		GUID GetAcceptExSockaddrsGuid = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD ByteCount = 0;

		if (WSAIoctl(ListenSocket->GetNative(), SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GetAcceptExSockaddrsGuid, sizeof(GetAcceptExSockaddrsGuid),
			&GetAcceptExSockaddrsFunc, sizeof(GetAcceptExSockaddrsFunc),
			&ByteCount, nullptr, nullptr) == SOCKET_ERROR)
		{
			FSocketUtility::ReportLastErrorCode();
			return false;
		}
	}

	Address.SetIPv4(INADDR_ANY);
	Address.SetPort(Port);

	if (!ListenSocket->Bind(Address))
	{
		return false;
	}

	if (!ListenSocket->Listen(SOMAXCONN))
	{
		return false;
	}

	if (!Iocp->Register(ListenSocket, &ListenSocket))
	{
		return false;
	}

	bRunning = true;

	for (i32 i = 0; i < ThreadCount; ++i)
	{
		WorkerThreads.emplace_back(&FChatServer::WorkerFunction, this);
	}

	const i32 PhysicalCoreCount = FPlatformWindowsUtility::GetPhysicalCoreCount();
	const i32 AcceptCount = PhysicalCoreCount > 0 ? PhysicalCoreCount * 2 : AcceptCountFallback;

	for (i32 i = 0; i < AcceptCount; ++i)
	{
		PostAccept();
	}

	return true;
}

void FChatServer::Cleanup()
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;

	{
		std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};
		for (auto& Connection : ConnectionMap | std::views::values)
		{
			Connection->Socket->Close();
		}

		ConnectionMap.clear();
	}

	if (ListenSocket && ListenSocket->IsValid())
	{
		ListenSocket->Close();
	}

	for (auto& WorkerThread : WorkerThreads)
	{
		if (WorkerThread.joinable())
		{
			WorkerThread.join();
		}
	}
}

bool FChatServer::PostAccept()
{
	std::shared_ptr<FTcpSocket> AcceptSocket = FSocketUtility::CreateIPv4TCP();
	if (!AcceptSocket || !AcceptSocket->IsValid())
	{
		return false;
	}

	constexpr DWORD AddressSize = sizeof(sockaddr_in) + 16;
	constexpr DWORD AddressSizeForData = (sizeof(sockaddr_in) + 16) * 2;

	FIocpContext* IocpContext = new FIocpContext{EIocpOperationType::Accept, AcceptSocket};
	IocpContext->Buffer.resize(AddressSizeForData);

	DWORD ByteCount;

	BOOL bAcceptEx = AcceptExFunc(ListenSocket->GetNative(), AcceptSocket->GetNative(), IocpContext->Buffer.data(),
		0, AddressSize, AddressSize, &ByteCount, &IocpContext->Overlapped);

	if (bAcceptEx == FALSE)
	{
		const i32 ErrorCode = FSocketUtility::GetLastErrorCode();
		if (ErrorCode != ERROR_IO_PENDING)
		{
			delete IocpContext;

			FSocketUtility::ReportErrorCode(ErrorCode);
			return false;
		}
	}

	return true;
}

bool FChatServer::PostSend(FIocpContext& SendContext, u32& ByteCount, u32 Flags)
{
	if (!SendContext.Socket || !SendContext.Socket->IsValid())
	{
		return false;
	}

	if (SendContext.Buffer.empty())
	{
		return false;
	}

	WSABUF Buffer;
	Buffer.buf = reinterpret_cast<CHAR*>(SendContext.Buffer.data());
	Buffer.len = static_cast<ULONG>(SendContext.Buffer.size());

	DWORD LocalByteCount = 0;
	DWORD LocalFlags = Flags;

	const int ErrorCode = WSASend(SendContext.Socket->GetNative(), &Buffer, 1, &LocalByteCount, LocalFlags, &SendContext.Overlapped, nullptr);

	if (ErrorCode == 0)
	{
		ByteCount = LocalByteCount;
		return true;
	}

	if (ErrorCode == SOCKET_ERROR && FSocketUtility::GetLastErrorCode() == WSA_IO_PENDING)
	{
		ByteCount = LocalByteCount;
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

bool FChatServer::PostRecv(FIocpContext& RecvContext, u32& ByteCount, u32 Flags)
{
	if (!RecvContext.Socket || !RecvContext.Socket->IsValid())
	{
		return false;
	}

	if (RecvContext.Buffer.empty())
	{
		return false;
	}

	WSABUF Buffer;
	Buffer.buf = reinterpret_cast<CHAR*>(RecvContext.Buffer.data());
	Buffer.len = static_cast<ULONG>(RecvContext.Buffer.size());

	DWORD LocalByteCount = 0;
	DWORD LocalFlags = Flags;

	const int ErrorCode = WSARecv(RecvContext.Socket->GetNative(), &Buffer, 1, &LocalByteCount, &LocalFlags, &RecvContext.Overlapped, nullptr);

	if (ErrorCode == 0)
	{
		ByteCount = LocalByteCount;
		return true;
	}

	if (ErrorCode == SOCKET_ERROR && FSocketUtility::GetLastErrorCode() == WSA_IO_PENDING)
	{
		ByteCount = LocalByteCount;
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

void FChatServer::HandleAccept(FIocpContext& AcceptContext)
{
	std::shared_ptr<FTcpSocket> Socket = AcceptContext.Socket;
	if (!Socket || !Socket->IsValid())
	{
		PostAccept();
		return;
	}

	sockaddr_in* LocalSockaddr;
	sockaddr_in* RemoteSockaddr;

	int LocalSockaddrLength;
	int RemoteSockaddrLength;

	constexpr DWORD SockaddrSize = sizeof(sockaddr_in) + 16;

	GetAcceptExSockaddrsFunc(AcceptContext.Buffer.data(), 0,
		SockaddrSize, SockaddrSize,
		reinterpret_cast<sockaddr**>(&LocalSockaddr), &LocalSockaddrLength,
		reinterpret_cast<sockaddr**>(&RemoteSockaddr), &RemoteSockaddrLength);

	SOCKET NativeListenSocket = ListenSocket->GetNative();

	// TODO: which class to put this?
	setsockopt(Socket->GetNative(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<const char*>(&NativeListenSocket), sizeof(SOCKET));

	FSocketAddress RemoteAddress{};
	memcpy(&RemoteAddress.Storage, RemoteSockaddr, sizeof(sockaddr_in));

	std::shared_ptr<FConnection> Connection = std::make_shared<FConnection>(Socket, "PLACEHOLDER", AcceptContext.Address);

	{
		std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};
		ConnectionMap[Socket->GetNative()] = Connection;
	}

	if (!Iocp->Register(Socket, reinterpret_cast<void*>(Socket->GetNative())))
	{
		Disconnect(Connection);
		PostAccept();
		return;
	}

	FIocpContext* RecvContext = new FIocpContext{EIocpOperationType::Recv, Socket};
	RecvContext->Buffer.resize(1024);

	u32 ByteCount = 0;
	if (!PostRecv(*RecvContext, ByteCount))
	{
		Disconnect(Connection);
		delete RecvContext;
	}

	PostAccept();
}

void FChatServer::HandleSend(FIocpContext& SendContext)
{
}

void FChatServer::HandleRecv(FIocpContext& RecvContext)
{
	std::shared_ptr<FTcpSocket> Socket = RecvContext.Socket;
	if (!Socket || !Socket->IsValid())
	{
		return;
	}

	// TODO
}

void FChatServer::Disconnect(std::shared_ptr<FConnection> Connection)
{
	std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};

	// NOTE: or change bConnected to atomic and CAS?

	if (!Connection || !Connection->bConnected)
	{
		return;
	}

	if (!Connection->Socket || !Connection->Socket->IsValid())
	{
		return;
	}

	Connection->Socket->Close();
	Connection->bConnected = false;

	ConnectionMap.erase(Connection->Socket->GetNative());
}

void FChatServer::WorkerFunction()
{
	std::vector<OVERLAPPED_ENTRY> Entries(64);
	u32 EntryCount;

	while (bRunning)
	{
		if (!Iocp || !Iocp->IsValid())
		{
			break;
		}

		if (!Iocp->Dequeue(Entries, EntryCount, std::chrono::milliseconds(100)))
		{
			continue;
		}

		for (u32 EntryIndex = 0; EntryIndex < EntryCount; ++EntryIndex)
		{
			const OVERLAPPED_ENTRY& Entry = Entries[EntryIndex];
			if (!Entry.lpOverlapped)
			{
				continue;
			}

			FIocpContext* IocpContext = reinterpret_cast<FIocpContext*>(Entry.lpOverlapped);

			switch (IocpContext->OperationType)
			{
			case EIocpOperationType::Accept:
				HandleAccept(*IocpContext);
				break;

			case EIocpOperationType::Recv:
				HandleRecv(*IocpContext);
				break;

			case EIocpOperationType::Send:
				HandleSend(*IocpContext);
				break;
			}

			delete IocpContext;
		}
	}
}
