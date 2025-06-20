﻿#include "ChatServer.h"

#include "ChatConnection.h"
#include "ChatMessage.h"
#include "Iocp.h"
#include "IocpContext.h"
#include "TcpSocket.h"

LPFN_ACCEPTEX AcceptExFunc = nullptr;
LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrsFunc = nullptr;

FChatServer::FChatServer():
	bRunning{false}
{
}

FChatServer::~FChatServer()
{
	Cleanup();
}

bool FChatServer::Startup(u16 Port, i32 ThreadCount, i32 AcceptCount)
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

	if (!Iocp->Register(ListenSocket, reinterpret_cast<void*>(ListenSocket->GetNative())))
	{
		return false;
	}

	bRunning = true;

	for (i32 i = 0; i < ThreadCount; ++i)
	{
		WorkerThreads.emplace_back(&FChatServer::WorkerFunction, this);
	}

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

	for (auto& WorkerThread : WorkerThreads)
	{
		if (WorkerThread.joinable())
		{
			WorkerThread.join();
		}
	}

	{
		std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};
		for (const std::shared_ptr<FChatConnection>& Connection : ConnectionMap | std::views::values)
		{
			Connection->Socket->Close();
		}

		ConnectionMap.clear();
	}

	if (ListenSocket && ListenSocket->IsValid())
	{
		ListenSocket->Close();
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

	if (!Iocp)
	{
		return false;
	}

	return Iocp->Send(SendContext.Socket, &SendContext.Overlapped, SendContext.Buffer.data(), static_cast<u32>(SendContext.Buffer.size()), ByteCount, Flags);
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

	if (!Iocp)
	{
		return false;
	}

	return Iocp->Recv(RecvContext.Socket, &RecvContext.Overlapped, RecvContext.Buffer.data(), static_cast<u32>(RecvContext.Buffer.size()), ByteCount, Flags);
}

void FChatServer::HandleAccept(FIocpContext& AcceptContext)
{
	std::shared_ptr<FTcpSocket> Socket = AcceptContext.Socket;
	if (!Socket || !Socket->IsValid())
	{
		PostAccept();
		return;
	}

	SOCKET NativeListenSocket = ListenSocket->GetNative();

	sockaddr_in* LocalSockaddr;
	sockaddr_in* RemoteSockaddr;

	int LocalSockaddrLength;
	int RemoteSockaddrLength;

	constexpr DWORD SockaddrSize = sizeof(sockaddr_in) + 16;

	GetAcceptExSockaddrsFunc(AcceptContext.Buffer.data(), 0,
		SockaddrSize, SockaddrSize,
		reinterpret_cast<sockaddr**>(&LocalSockaddr), &LocalSockaddrLength,
		reinterpret_cast<sockaddr**>(&RemoteSockaddr), &RemoteSockaddrLength);

	FSocketAddress RemoteAddress{};
	memcpy(&RemoteAddress.Storage, RemoteSockaddr, sizeof(sockaddr_in));

	std::shared_ptr<FChatConnection> Connection = std::make_shared<FChatConnection>();
	Connection->Socket = Socket;
	Connection->UserName = "PLACEHOLDER";
	Connection->Address = RemoteAddress;

	{
		const i32 ReturnCode = setsockopt(Socket->GetNative(), SOL_SOCKET,
			SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<const char*>(&NativeListenSocket), sizeof(NativeListenSocket));

		if (ReturnCode == SOCKET_ERROR)
		{
			FSocketUtility::ReportLastErrorCode();

			Disconnect(Connection);
			PostAccept();
			return;
		}
	}

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
	RecvContext->Buffer.resize(sizeof(FChatMessage));

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

	constexpr usize MessageSize = sizeof(FChatMessage);
	if (RecvContext.Buffer.size() < MessageSize)
	{
		return;
	}

	FChatMessage Message;
	memcpy(&Message, RecvContext.Buffer.data(), MessageSize);

	std::shared_ptr<FChatConnection> Connection;

	{
		std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};

		const auto It = ConnectionMap.find(Socket->GetNative());
		if (It == ConnectionMap.end())
		{
			return;
		}

		Connection = It->second;
	}

	if (!Connection->IsConnected())
	{
		return;
	}

	switch (Message.Type)
	{
	case EChatMessageType::Login:
	{
		constexpr usize MaxUserNameSize = sizeof(Message.UserName);
		usize UserNameSize = Message.UserNameSize;

		if (UserNameSize == 0 || UserNameSize > MaxUserNameSize)
		{
			UserNameSize = strnlen_s(Message.UserName, MaxUserNameSize);
		}

		Connection->UserName = UserNameSize > 0 ? std::string{Message.UserName, UserNameSize} : std::string{"UNKNOWN"};

		FChatMessage LoginMessage;
		LoginMessage.Type = EChatMessageType::System;

		const std::string LoginString = std::format("{}님이 채팅에 입장하셨습니다", Connection->UserName);
		strncpy_s(LoginMessage.Data, sizeof(LoginMessage.Data), LoginString.c_str(), sizeof(LoginMessage.Data) - 1);

		std::cout << "[LOGIN] " << Connection->UserName << ": " << Connection->Address.ToIPv4String(true) << '\n';

		SendAll(LoginMessage);

		FChatMessage WelcomeMessage;
		WelcomeMessage.Type = EChatMessageType::System;
		strcpy_s(WelcomeMessage.Data, "채팅에 오신 것을 환영합니다");

		SendTo(Connection, WelcomeMessage);

		break;
	}

	case EChatMessageType::Logout:
	{
		FChatMessage LogoutMessage;
		LogoutMessage.Type = EChatMessageType::System;

		const std::string LogoutString = std::format("{}님이 채팅에서 나가셨습니다", Connection->UserName);
		strncpy_s(LogoutMessage.Data, LogoutString.c_str(), sizeof(LogoutMessage.Data) - 1);

		std::cout << "[LOGOUT] " << Connection->UserName << ": " << Connection->Address.ToIPv4String(true) << '\n';

		SendAll(LogoutMessage);

		Disconnect(Connection);

		break;
	}

	case EChatMessageType::Data:
	{
		std::cout << "[DATA] " << Connection->UserName << ": " << Connection->Address.ToIPv4String(true) << '\n';

		SendAll(Message);

		break;
	}

	default:
		break;
	}

	FIocpContext* NextRecvContext = new FIocpContext{EIocpOperationType::Recv, Socket};
	NextRecvContext->Buffer.resize(MessageSize);

	u32 ByteCount = 0;
	if (!PostRecv(*NextRecvContext, ByteCount))
	{
		Disconnect(Connection);
		delete NextRecvContext;
	}
}

// maybe just change paramter to SOCKET?
void FChatServer::Disconnect(std::shared_ptr<FChatConnection> Connection)
{
	if (!Connection)
	{
		return;
	}

	if (!Connection->TryDisconnect())
	{
		return;
	}

	SOCKET NativeSocket = INVALID_SOCKET;

	if (Connection->Socket && Connection->Socket->IsValid())
	{
		NativeSocket = Connection->Socket->GetNative();
		Connection->Socket->Close();
	}

	Connection->SetDisconnected();

	if (NativeSocket != INVALID_SOCKET)
	{
		std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};
		ConnectionMap.erase(NativeSocket);
	}
}

void FChatServer::SendTo(const std::shared_ptr<FChatConnection>& Connection, const FChatMessage& Message)
{
	if (!Connection || !Connection->IsConnected())
	{
		return;
	}

	FIocpContext* SendContext = new FIocpContext{EIocpOperationType::Send, Connection->Socket};
	SendContext->Buffer.resize(sizeof(Message));
	memcpy(SendContext->Buffer.data(), &Message, sizeof(Message));

	u32 ByteCount = 0;
	if (!PostSend(*SendContext, ByteCount))
	{
		Disconnect(Connection);
		delete SendContext;
	}
}

void FChatServer::SendAll(const FChatMessage& Message)
{
	std::vector<std::shared_ptr<FChatConnection>> LocalConnections;
	LocalConnections.reserve(ConnectionMap.size());

	{
		std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};

		for (const std::shared_ptr<FChatConnection>& Connection : ConnectionMap | std::views::values)
		{
			LocalConnections.emplace_back(Connection);
		}
	}

	// SendTo will check bConnected anyway
	for (const std::shared_ptr<FChatConnection>& Connection : LocalConnections)
	{
		SendTo(Connection, Message);
	}
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

			if (IocpContext->OperationType == EIocpOperationType::Recv && Entry.dwNumberOfBytesTransferred != sizeof(FChatMessage))
			{
				std::shared_ptr<FTcpSocket> Socket = IocpContext->Socket;
				std::shared_ptr<FChatConnection> Connection;

				{
					std::scoped_lock<std::mutex> ConnectionMapLock{ConnectionMapMutex};

					const auto It = ConnectionMap.find(Socket->GetNative());
					if (It != ConnectionMap.end())
					{
						Connection = It->second;
					}
				}

				if (Connection)
				{
					Disconnect(Connection);
				}

				delete IocpContext;
				continue;
			}

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

	std::cout << "워커 스레드 종료\n";
}
