#include "SocketUDP.h"

#include "SocketAddress.h"
#include "SocketUtility.h"

FSocketUDP::FSocketUDP(SOCKET InSocket) :
	Socket(InSocket)
{
}

FSocketUDP::~FSocketUDP()
{
	closesocket(Socket);
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FSocketUDP::Bind(const FSocketAddress& Address)
{
	const i32 ErrorCode = bind(Socket, &Address.SockAddr, sizeof(Address.SockAddr));
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FSocketUDP::SendTo(const u8* Data, i32 Count, i32& ByteCount, const FSocketAddress& Address)
{
	ByteCount = sendto(Socket, reinterpret_cast<const char*>(Data), Count, 0, &Address.SockAddr, sizeof(Address.SockAddr));

	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FSocketUDP::RecvFrom(u8* Data, i32 Count, i32& ByteCount, FSocketAddress& OutAddress)
{
	i32 Length = sizeof(OutAddress.SockAddr);

	ByteCount = recvfrom(Socket, reinterpret_cast<char*>(Data), Count, 0, &OutAddress.SockAddr, &Length);

	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}
