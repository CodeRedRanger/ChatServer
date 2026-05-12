#pragma once
//these includes were not needed, even though research said they were needed
//#include <windows.h>
//#include <shellapi.h>
#include <iostream>
#include <fstream>
#include "AuthManager.h"
#include <sstream>
#include <chrono>
#include <iomanip>



namespace Logger
{
	void LogPublicMessage(SOCKET client, const std::string& message);
	void LogCommand(SOCKET client, const std::string& commandStr);

	std::string GetTimestamp();
	std::string GetCurrentUser(SOCKET client);
	std::string GetPublicChatLog();
}