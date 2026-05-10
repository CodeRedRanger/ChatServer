#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include "ClientHandler.h"



void Client::ClientCode(void)
{
	//Socket
	SOCKET ComSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ComSocket == INVALID_SOCKET)
	{
		printf("DEBUG// Socket function incorrect\n");
		return;
	}
	else
	{
		printf("DEBUG// I used the socket function\n");
	}

	//Connect
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(31337);

	int result = connect(ComSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Connect function incorrect\n");
		return;
	}
	else
	{
		printf("DEBUG// I used the Connect function\n");
	}

	//Communication
	char sendbuffer[30];
	memset(sendbuffer, 0, 30);
	strcpy(sendbuffer, "I'm a message from the client");
	uint8_t size = strlen(sendbuffer);

	result = TCPFraming::tcp_send_whole(ComSocket, (char*)&size, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("DEBUG// send is incorrect\n");
		return;

	}
	else
	{
		printf("DEBUG// I used the send function\n");
	}



	result = TCPFraming::tcp_send_whole(ComSocket, sendbuffer, size);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("DEBUG// send is incorrect\n");
		return;

	}
	else
	{
		printf("DEBUG// I used the send function\n");
	}

	printf("DEBUG// I sent a message to the server\n");

	//need to adjust this so you can get out of loop by typing exit
	while (true)
	{
		char sendbuffer[256];

		fgets(sendbuffer, 256, stdin);

		uint8_t size = strlen(sendbuffer);

		TCPFraming::tcp_send_whole(ComSocket, (char*)&size, 1);
		TCPFraming::tcp_send_whole(ComSocket, sendbuffer, size);
	}

	// close sockets
	shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);
}
