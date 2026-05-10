#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include "ClientHandler.h"


void Client::Stop(SOCKET socket)
{
	if (socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}
}

int Client::InitCommSocket(SOCKET& comSocket, uint16_t port, const char* address)
{
	// Socket
	comSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (comSocket == INVALID_SOCKET)
	{
		return SETUP_ERROR;
	}

	// Connect
	sockaddr_in serverAddr = {};

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	if (InetPtonA(AF_INET, address, &serverAddr.sin_addr) != 1)
	{
		closesocket(comSocket);
		return ADDRESS_ERROR;
	}

	int result = connect(
		comSocket,
		(SOCKADDR*)&serverAddr,
		sizeof(serverAddr)
	);

	if (result == SOCKET_ERROR)
	{
		closesocket(comSocket);
		return CONNECT_ERROR;
	}

	printf("DEBUG// I used the Connect function\n");
	return SUCCESS;
}


void Client::ClientCode(void)
{
	SOCKET ComSocket = INVALID_SOCKET;

	int result = InitCommSocket(
		ComSocket,
		31337,
		(char*)"127.0.0.1"
	);

	if (result != SUCCESS)
	{
		printf("DEBUG// Failed to initialize communication socket\n");
		return;
	}

	printf("DEBUG// Connected to server\n");



	//Communication
	char sendbuffer[30];
	memset(sendbuffer, 0, 30);
	strcpy(sendbuffer, "I'm a message from the client");

	/*
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
	*/

	result = TCPFraming::sendFrame(
		ComSocket,
		sendbuffer,
		(uint16_t)strlen(sendbuffer)
	);

	if (result != SUCCESS)
	{
		printf("DEBUG// send failed\n");
		return;
	}

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


	char recvbuffer[257] = {};

	result = TCPFraming::readFrame(
		ComSocket,
		recvbuffer,
		sizeof(recvbuffer)
	);

	if (result == SUCCESS)
	{
		printf("%s\n", recvbuffer);
	}



	//need to adjust this so you can get out of loop by typing exit
	while (true)
	{
		char sendbuffer[256] = {};

		fgets(sendbuffer, sizeof(sendbuffer), stdin);

		sendbuffer[strcspn(sendbuffer, "\n")] = '\0';

		if (strcmp(sendbuffer, "exit") == 0)
		{
			break;
		}

		result = TCPFraming::sendFrame(
			ComSocket,
			sendbuffer,
			(uint16_t)strlen(sendbuffer)
		);

		if (result != SUCCESS)
		{
			printf("DEBUG// send failed\n");
			break;
		}
	}

	// close sockets
	Stop(ComSocket);
}
