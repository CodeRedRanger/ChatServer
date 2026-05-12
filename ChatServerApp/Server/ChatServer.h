#pragma once
#include "MessageHandler.h"
//for message box A
//#include <Windows.h>
#include <limits>



namespace Server
{

	void ServerCode(void);
	int InitListeningSocket(uint16_t port, SOCKET& listenSocket, int capacity); 
	void Stop(SOCKET socket);
	void DisconnectClient(SOCKET client, fd_set& masterSet, int& currentClients);

}