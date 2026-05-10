// ServerClientConsole.cpp 
// By Phillip Croff
// message format is 1-byte for the message length followed by the message
//  ex:   6hello\0

#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include <winsock2.h>
//#include <ws2tcpip.h>                         // only need if you use inet_pton
#pragma comment(lib,"Ws2_32.lib")

#include <iostream>
#include "./Client/ClientHandler.h"
#include "./Server/ChatServer.h"



int main()
{
	WSADATA wsadata;
	WSAStartup(WINSOCK_VERSION, &wsadata);

	int choice;
	do
	{
		printf("Would you like to Create a Server or Client?\n");
		printf("1> Server\n");
		printf("2> Client\n");
		std::cin >> choice;
	} while (choice != 1 && choice != 2);

	//Server
	if (choice == 1)
	{		
		Server::ServerCode();
	}

	//Client
	if (choice == 2)
	{
		Client::ClientCode();
	}

	return WSACleanup();
}
