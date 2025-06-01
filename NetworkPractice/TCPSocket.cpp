#include "TCPSocket.h"

#include "SocketAddress.h"
#include "SocketUtility.h"

FTCPSocket::FTCPSocket(SOCKET InSocket) :
	TSocket(InSocket)
{
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FTCPSocket::Connect(const FSocketAddress& Address)
{
	const i32 ErrorCode = connect(Socket, &Address.SockAddr, sizeof(Address.SockAddr));
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FTCPSocket::Listen(i32 MaxBacklog)
{
	const i32 ErrorCode = listen(Socket, MaxBacklog);
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
std::shared_ptr<FTCPSocket> FTCPSocket::Accept(FSocketAddress& OutAddress)
{
	i32 Length = sizeof(OutAddress.SockAddr);
	SOCKET NewSocket = accept(Socket, &OutAddress.SockAddr, &Length);

	if (NewSocket != INVALID_SOCKET)
	{
		return std::make_shared<FTCPSocket>(NewSocket);
	}

	FSocketUtility::ReportLastErrorCode();
	return nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FTCPSocket::Send(const u8* Data, i32 BufferSize, i32& ByteCount, i32 flags)
{
	ByteCount = send(Socket, reinterpret_cast<const char*>(Data), BufferSize, flags);
	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FTCPSocket::Recv(u8* Data, i32 BufferSize, i32& ByteCount, i32 flags)
{
	ByteCount = recv(Socket, reinterpret_cast<char*>(Data), BufferSize, flags);
	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FTCPSocket::SetNoDelay(bool bNoDelay)
{
	const i32 Value = bNoDelay ? 1 : 0;

	const i32 ErrorCode = setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&Value), sizeof(Value));
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}
