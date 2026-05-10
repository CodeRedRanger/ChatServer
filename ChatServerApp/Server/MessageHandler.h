#pragma once
#include <iostream>
#include <sstream>
#include "AuthManager.h"
#include "TCPFraming.h"


namespace MessageHandler
{
	
	void HandleCommand(SOCKET client, SOCKET listenSocket, fd_set& masterSet, const char cmdChar, const char* msg); 
	void HandleRegister(SOCKET client, const std::string& msg);
	void HandleLogin(SOCKET client, const std::string& msg);
	void HandleGetHelp(SOCKET client, const char cmdChar); 

}
