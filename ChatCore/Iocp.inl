#pragma once

#include "SocketUtility.h"

template <typename T>
bool FIocp::Register(const std::shared_ptr<T>& Socket, void* Key)
{
	HANDLE NewHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket->GetNative()), Handle, reinterpret_cast<ULONG_PTR>(Key), 0);
	if (NewHandle != INVALID_HANDLE_VALUE)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}
