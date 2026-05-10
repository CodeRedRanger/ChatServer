#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include "ClientHandler.h"


//gracefully closes socket by first shutting down both send and receive operations, then closing the socket handle.
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
	// Creates Communcation Socket
	comSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for socket creation error
	if (comSocket == INVALID_SOCKET)
	{
		return SETUP_ERROR;
	}

	// Connect
	sockaddr_in serverAddr = {};

	//sets family to AF_INET (IPv4), address to the provided address converted from string to binary form using InetPtonA, and port in network byte order (htons = host to network short)
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	// Convert address string to binary form and store in serverAddr.sin_addr, if it fails, closes the socket and returns an error code.
	if (InetPtonA(AF_INET, address, &serverAddr.sin_addr) != 1)
	{
		closesocket(comSocket);
		return ADDRESS_ERROR;
	}

	// Connect to the server
	int result = connect(comSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	// If it fails, closes the socket and returns an error code. Reasons this could fail include if the server is not running, 
	// if the address or port is incorrect, or if there are network issues preventing the connection, firewall.
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

	//hardcoded address and port for testing, can change later to user input or config file
	int result = InitCommSocket(ComSocket, 31337,(char*)"127.0.0.1");

	if (result != SUCCESS)
	{
		printf("DEBUG// Failed to initialize communication socket\n");
		return;
	}

	printf("DEBUG// Connected to server\n");



	//Communication
	//can receive 256 + 1 for size byte, but buffer needs to be 257 to hold null terminator for printf
	char recvbuffer[257] = {};

	while (true)
	{
		result = TCPFraming::readFrame(ComSocket, recvbuffer, sizeof(recvbuffer));

		if (result == SUCCESS)
		{
			printf("%s\n", recvbuffer);
		}
		else if (result == SHUTDOWN)
		{
			printf("Server closed connection.\n");
			break;
		}
		else if (result == DISCONNECT)
		{
			printf("Connection lost.\n");
			break;
		}
		else if (result == PARAMETER_ERROR)
		{
			printf("Warning: malformed message received.\n");
			continue; // NOT fatal
		}
		else
		{
			printf("Unknown error (%d)\n", result);
			break;
		}
	}

	//need to adjust this so you can get out of loop by typing exit
	while (true)
	{
		//input buffer
		char sendbuffer[256] = {};

		//reads full line from keyboard input into sendbuffer, including spaces, until newline is encountered or buffer limit is reached.
		fgets(sendbuffer, sizeof(sendbuffer), stdin);

		//remove the newline character
		sendbuffer[strcspn(sendbuffer, "\n")] = '\0';


		//FIX SO LOGS OUT USER, DO LATER WHEN DOING LOGOUT FUNCTIONALITY
		//this doesn't seem to work, you don't get disconnected when typing this, just breaks out of loops
		if (strcmp(sendbuffer, "exit") == 0)
		{
			break;
		}

		result = TCPFraming::sendFrame(ComSocket,sendbuffer, (uint16_t)strlen(sendbuffer));

		if (result != SUCCESS)
		{
			//simple succeed or fail is find for sending messages
			printf("Connection lost while sending.\n");
			break;
		}
	}

	// close sockets
	Stop(ComSocket);
}
