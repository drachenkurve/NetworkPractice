#pragma once

#include "SocketUtility.h"

template <typename T>
TSocket<T>::TSocket(SOCKET InSocket):
	Socket(InSocket)
{
}

template <typename T>
TSocket<T>::~TSocket()
{
	closesocket(Socket);
}

// ReSharper disable once CppMemberFunctionMayBeConst
template <typename T>
bool TSocket<T>::SetNonBlocking(bool bNonBlocking)
{
	u_long Value = bNonBlocking ? 1 : 0;

	const i32 ErrorCode = ioctlsocket(Socket, FIONBIO, &Value);
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
template <typename T>
bool TSocket<T>::SetBroadcast(bool bBroadcast)
{
	const i32 Value = bBroadcast ? 1 : 0;

	const i32 ErrorCode = setsockopt(Socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&Value), sizeof(Value));
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
template <typename T>
bool TSocket<T>::SetSendBufferSize(i32 Size, i32& NewSize)
{
	const i32 ErrorCode = setsockopt(Socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&Size), sizeof(Size));

	i32 SizeOfSize = sizeof(NewSize);
	getsockopt(Socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&NewSize), &SizeOfSize);

	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
template <typename T>
bool TSocket<T>::SetRecvBufferSize(i32 Size, i32& NewSize)
{
	const i32 ErrorCode = setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&Size), sizeof(Size));

	i32 SizeOfSize = sizeof(NewSize);
	getsockopt(Socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&NewSize), &SizeOfSize);

	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportErrorCode(ErrorCode);
	return false;
}
