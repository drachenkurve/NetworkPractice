﻿#pragma once

#include "SocketAddress.h"
#include "SocketUtility.h"

template <typename T>
TSocket<T>::TSocket(SOCKET InSocket):
	Socket(InSocket)
{
}

template <typename T>
TSocket<T>::~TSocket()
{
	if (Socket != INVALID_SOCKET)
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
	}
}

template <typename T>
bool TSocket<T>::IsValid() const
{
	return Socket != INVALID_SOCKET;
}

// ReSharper disable once CppMemberFunctionMayBeConst
template <typename T>
bool TSocket<T>::Bind(const FSocketAddress& Address)
{
	const i32 ErrorCode = bind(Socket, reinterpret_cast<const sockaddr*>(&Address.Storage), Address.GetStorageSize());
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
template <typename T>
bool TSocket<T>::Shutdown(i32 Mode)
{
	const i32 ErrorCode = shutdown(Socket, Mode);
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

template <typename T>
bool TSocket<T>::Close()
{
	if (Socket == INVALID_SOCKET)
	{
		return false;
	}

	const i32 ErrorCode = closesocket(Socket);

	// socket is not valid anyway
	Socket = INVALID_SOCKET;

	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
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

	FSocketUtility::ReportLastErrorCode();
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

	FSocketUtility::ReportLastErrorCode();
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

	FSocketUtility::ReportLastErrorCode();
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

	FSocketUtility::ReportLastErrorCode();
	return false;
}

template <typename T>
SOCKET TSocket<T>::GetNative() const
{
	return Socket;
}
