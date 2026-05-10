#pragma once

#include <ws2tcpip.h>  
#include <iostream>
#include "../Server/TCPFraming.h"



namespace Client
{
	void ClientCode(void);
	int InitCommSocket(SOCKET& comSocket, uint16_t port, const char* address);
	void Stop(SOCKET socket); 

}


