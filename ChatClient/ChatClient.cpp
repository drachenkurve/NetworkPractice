#include "ChatClient.h"

#include "ChatMessage.h"
#include "Iocp.h"
#include "IocpContext.h"
#include "TcpSocket.h"

FChatClient::FChatClient(HANDLE InStopEvent) :
	bRunning{false}, StopEvent{InStopEvent}
{
}

FChatClient::~FChatClient()
{
	Cleanup();
}

bool FChatClient::Startup(std::string_view IPv4String)
{
	if (bRunning)
	{
		return false;
	}

	Iocp = std::make_unique<FIocp>(1);
	if (!Iocp || !Iocp->IsValid())
	{
		return false;
	}

	Socket = FSocketUtility::CreateIPv4TCP();
	if (!Socket || !Socket->IsValid())
	{
		return false;
	}

	FSocketAddress Address{};
	Address.FromIPv4String(IPv4String);

	if (!Socket->Connect(Address))
	{
		return false;
	}

	if (!Iocp->Register(Socket, reinterpret_cast<void*>(Socket->GetNative())))
	{
		return false;
	}

	bRunning = true;

	RecvThread = std::thread{&FChatClient::RecvFunction, this};

	return true;
}

void FChatClient::Cleanup()
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;

	if (RecvThread.joinable())
	{
		RecvThread.join();
	}

	if (Socket && Socket->IsValid())
	{
		Socket->Shutdown(SD_BOTH);
		Socket->Close();
	}

	Socket.reset();
	Iocp.reset();
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FChatClient::PostSend(const FChatMessage& Message)
{
	if (!Socket || !Socket->IsValid())
	{
		return false;
	}

	// ensure single message
	std::scoped_lock<std::mutex> SendLock{SendMutex};

	FIocpContext* SendContext = new FIocpContext{EIocpOperationType::Send, Socket};
	SendContext->Buffer.resize(sizeof(FChatMessage));
	memcpy(SendContext->Buffer.data(), &Message, sizeof(FChatMessage));

	u32 ByteCount = 0;
	if (!Iocp->Send(Socket, &SendContext->Overlapped, SendContext->Buffer.data(), static_cast<u32>(SendContext->Buffer.size()), ByteCount))
	{
		delete SendContext;
		return false;
	}

	return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FChatClient::PostRecv()
{
	if (!Socket || !Socket->IsValid())
	{
		return false;
	}

	FIocpContext* RecvContext = new FIocpContext{EIocpOperationType::Recv, Socket};
	RecvContext->Buffer.resize(sizeof(FChatMessage));

	u32 ByteCount = 0;
	if (!Iocp->Recv(Socket, &RecvContext->Overlapped, RecvContext->Buffer.data(), static_cast<u32>(RecvContext->Buffer.size()), ByteCount))
	{
		delete RecvContext;
		return false;
	}

	return true;
}

void FChatClient::HandleSend(FIocpContext& SendContext)
{
}

void FChatClient::HandleRecv(FIocpContext& RecvContext)
{
	if (!Socket || !Socket->IsValid())
	{
		bRunning = false;

		if (StopEvent != INVALID_HANDLE_VALUE)
		{
			SetEvent(StopEvent);
		}

		return;
	}

	constexpr usize MessageSize = sizeof(FChatMessage);
	if (RecvContext.Buffer.size() < MessageSize)
	{
		return;
	}

	FChatMessage Message;
	memcpy(&Message, RecvContext.Buffer.data(), MessageSize);

	switch (Message.Type)
	{
	case EChatMessageType::System:
	{
		constexpr usize MaxDataSize = sizeof(Message.Data);
		usize DataSize = Message.DataSize;

		if (DataSize == 0 || DataSize > MaxDataSize)
		{
			DataSize = strnlen_s(Message.Data, MaxDataSize);
		}

		std::string DataStr = DataSize > 0 ? std::string{Message.Data, DataSize} : "BROKEN";

		std::cout << "[SYSTEM] " << DataStr << "\n";

		break;
	}

	case EChatMessageType::Data:
	{
		constexpr usize MaxUserNameSize = sizeof(Message.UserName);
		usize UserNameSize = Message.UserNameSize;

		if (UserNameSize == 0 || UserNameSize > MaxUserNameSize)
		{
			UserNameSize = strnlen_s(Message.UserName, MaxUserNameSize);
		}

		std::string UserName = UserNameSize > 0 ? std::string{Message.UserName, UserNameSize} : "UNKNOWN";

		constexpr usize MaxDataSize = sizeof(Message.Data);
		usize DataSize = Message.DataSize;

		if (DataSize == 0 || DataSize > MaxDataSize)
		{
			DataSize = strnlen_s(Message.Data, MaxDataSize);
		}

		std::string Data = DataSize > 0 ? std::string{Message.Data, DataSize} : "BROKEN";

		std::cout << "[" << UserName << "] " << Data << "\n";

		break;
	}

	default:
		break;
	}

	if (!PostRecv())
	{
		bRunning = false;

		if (StopEvent != INVALID_HANDLE_VALUE)
		{
			SetEvent(StopEvent);
		}
	}
}

void FChatClient::RecvFunction()
{
	std::vector<OVERLAPPED_ENTRY> Entries(8);
	u32 EntryCount;

	if (!PostRecv())
	{
		bRunning = false;

		if (StopEvent != INVALID_HANDLE_VALUE)
		{
			SetEvent(StopEvent);
		}
	}

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

		for (u32 i = 0; i < EntryCount; ++i)
		{
			OVERLAPPED_ENTRY& Entry = Entries[i];
			if (!Entry.lpOverlapped)
			{
				continue;
			}

			FIocpContext* Context = reinterpret_cast<FIocpContext*>(Entry.lpOverlapped);

			switch (Context->OperationType)
			{
			case EIocpOperationType::Recv:
				HandleRecv(*Context);
				break;
			case EIocpOperationType::Send:
				HandleSend(*Context);
				break;
			default:
				break;
			}

			delete Context;
		}
	}
}
