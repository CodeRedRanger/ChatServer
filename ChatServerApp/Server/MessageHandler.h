#pragma once
#include "TCPFraming.h"
#include "Logger.h"


namespace MessageHandler
{
	
	//bool for returning a signal to server for disconnections
	bool HandleCommand(SOCKET client, SOCKET listenSocket, fd_set& masterSet, const char cmdChar, const char* msg, int capacity); 

	void HandleRegister(SOCKET client, const std::string& msg, int capacity);
	void HandleLogin(SOCKET client, const std::string& msg);

	//bool for returning a signal for disconnections 
	bool HandleLogout(SOCKET client, const std::string& msg);
	void HandleGetHelp(SOCKET client, const char cmdChar); 

	//not sure about parameters below
	void HandleSend(SOCKET client, const std::string& msg);

	void HandleGetList(SOCKET client);
	void HandleGetLog(SOCKET client);

}
