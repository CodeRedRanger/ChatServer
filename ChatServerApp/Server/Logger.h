#pragma once
#include <iostream>
#include <fstream>
#include "AuthManager.h"
#include <sstream>
#include <chrono>
#include <iomanip>


namespace Logger
{
	void LogPublicMessage(SOCKET client, const std::string& message);
	//void LogError(const std::string& errorMessage);
	void LogClientMessage(const std::string& username, const std::string& message);
	//void LogClientConnection(const std::string& username);
	//void LogClientDisconnection(const std::string& username);
	void LogCommand(SOCKET client, const std::string& commandStr);
	//void LogAuthEvent(const std::string& username, const std::string& event);

	std::string GetTimestamp();
	std::string GetCurrentUser(SOCKET client);
	std::string GetPublicChatLog();
}