#include "LaunchWindows.h"

// Project Headers
#include "ChatClient.h"
#include "ChatMessage.h"
#include "SocketUtility.h"

HANDLE StopEvent = INVALID_HANDLE_VALUE;

BOOL WINAPI HandleConsoleCtrl(DWORD Signal)
{
	if (Signal == CTRL_C_EVENT)
	{
		std::cout << "Ctrl+C 감지\n";

		if (StopEvent != INVALID_HANDLE_VALUE)
		{
			SetEvent(StopEvent);
		}

		return TRUE;
	}

	return FALSE;
}

std::string UserName;
std::atomic<bool> bRunningInput;

void InputFunction(FChatClient& Client)
{
	while (bRunningInput)
	{
		// FIXME: I need to synchronize cout...
		//std::cout << "> ";

		std::string Input;
		std::getline(std::cin, Input);

		if (Input.empty())
		{
			continue;
		}

		FChatMessage Message{};
		Message.Type = EChatMessageType::Data;

		strncpy_s(Message.UserName, sizeof(Message.UserName), UserName.c_str(), _TRUNCATE);
		Message.UserNameSize = static_cast<u16>(strlen(Message.UserName));

		strncpy_s(Message.Data, sizeof(Message.Data), Input.c_str(), _TRUNCATE);
		Message.DataSize = static_cast<u16>(strlen(Message.Data));

		Client.PostSend(Message);
	}
}

void GuardedMain()
{
	FSocketUtility::Startup();

	StopEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
	SetConsoleCtrlHandler(HandleConsoleCtrl, TRUE);

	FChatClient Client{StopEvent};

	if (Client.Startup("127.0.0.1:7777"))
	{
		std::cout << "이름 입력: ";
		std::getline(std::cin, UserName);

		if (UserName.empty())
		{
			UserName = "ANONYMOUS";
		}

		FChatMessage LoginMessage{};
		LoginMessage.Type = EChatMessageType::Login;

		strncpy_s(LoginMessage.UserName, sizeof(LoginMessage.UserName), UserName.c_str(), _TRUNCATE);
		LoginMessage.UserNameSize = static_cast<u16>(strlen(LoginMessage.UserName));

		Client.PostSend(LoginMessage);

		bRunningInput = true;
		std::thread InputThread{InputFunction, std::ref(Client)};

		WaitForSingleObject(StopEvent, INFINITE);

		bRunningInput = false;

		if (InputThread.joinable())
		{
			InputThread.join();
		}

		FChatMessage LogoutMessage{};
		LogoutMessage.Type = EChatMessageType::Logout;

		strncpy_s(LogoutMessage.UserName, sizeof(LogoutMessage.UserName), UserName.c_str(), _TRUNCATE);
		LogoutMessage.UserNameSize = static_cast<u16>(strlen(LogoutMessage.UserName));

		Client.PostSend(LogoutMessage);
	}

	Client.Cleanup();

	CloseHandle(StopEvent);

	FSocketUtility::Cleanup();
}
