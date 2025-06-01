#include "UDPSocket.h"

#include "SocketAddress.h"
#include "SocketUtility.h"

FUDPSocket::FUDPSocket(SOCKET InSocket) :
	TSocket(InSocket)
{
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FUDPSocket::Bind(const FSocketAddress& Address)
{
	const i32 ErrorCode = bind(Socket, &Address.SockAddr, sizeof(Address.SockAddr));
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FUDPSocket::SendTo(const u8* Data, i32 Count, i32& ByteCount, const FSocketAddress& Address, i32 flags)
{
	ByteCount = sendto(Socket, reinterpret_cast<const char*>(Data), Count, flags, &Address.SockAddr, sizeof(Address.SockAddr));
	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FUDPSocket::RecvFrom(u8* Data, i32 Count, i32& ByteCount, FSocketAddress& OutAddress, i32 flags)
{
	i32 Length = sizeof(OutAddress.SockAddr);

	ByteCount = recvfrom(Socket, reinterpret_cast<char*>(Data), Count, flags, &OutAddress.SockAddr, &Length);
	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}
