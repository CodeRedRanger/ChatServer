#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include "ClientHandler.h"


//UDP
/*
void UDPDiscoveryThread(std::atomic<bool>& running)
{
	SOCKET udpRecv =
		socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (udpRecv == INVALID_SOCKET)
	{
		std::cout << "UDP socket failed\n";
		return;
	}

	BOOL opt = TRUE;

	DWORD timeout = 1000;

	setsockopt(
		udpRecv,
		SOL_SOCKET,
		SO_RCVTIMEO,
		(char*)&timeout,
		sizeof(timeout));

	sockaddr_in addr = {};

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(54000);

	if (bind(
		udpRecv,
		(sockaddr*)&addr,
		sizeof(addr)) == SOCKET_ERROR)
	{
		std::cout << "UDP bind failed\n";

		closesocket(udpRecv);
		return;
	}

	char buffer[256];


	while (running)
	{
		sockaddr_in senderAddr;
		int senderLen = sizeof(senderAddr);

		int result = recvfrom(
			udpRecv,
			buffer,
			sizeof(buffer) - 1,
			0,
			(sockaddr*)&senderAddr,
			&senderLen);

		if (result > 0)
		{
			buffer[result] = '\0';

			std::cout
				<< "UDP Discovery: "
				<< buffer
				<< "\n";
		}
	}


	closesocket(udpRecv);
}
*/

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

		int err = WSAGetLastError();
		if (err == WSAESHUTDOWN)
		{
			std::cout << "connect() : socket shutdown detected\n";
			closesocket(comSocket);
			return CONNECT_ERROR;
		}


		// For other errors, log the error code and return a generic connection error.
		std::cout << "connect() failed with error: " << err << "\n";
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



	//UDP
	/*
	std::atomic<bool> running = true;

	std::thread udpThread(
		UDPDiscoveryThread,
		std::ref(running));*/


	//TCP
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

		//this breaks the loop, but will handle better with log out. Could still use this as a way to close the GUI?
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
	/*
	running = false;
	if (udpThread.joinable())
	{
		udpThread.join();
	}*/
}
