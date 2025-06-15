#include "ChatConnection.h"

bool FChatConnection::IsConnected() const
{
	std::scoped_lock<std::mutex> StateLock{StateMutex};

	return State == EChatConnectionState::Connected;
}

EChatConnectionState FChatConnection::GetState() const
{
	std::scoped_lock<std::mutex> StateLock{StateMutex};

	return State;
}

bool FChatConnection::TryDisconnect()
{
	std::scoped_lock<std::mutex> StateLock{StateMutex};

	if (State != EChatConnectionState::Connected)
	{
		return false;
	}

	State = EChatConnectionState::Disconnecting;
	return true;
}

void FChatConnection::SetDisconnected()
{
	std::scoped_lock<std::mutex> StateLock{StateMutex};

	State = EChatConnectionState::Disconnected;
}
