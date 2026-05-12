#include "ChatServer.h"

/*

I've got to use the following:

1. SO_REUSEADDR:
• The SO_REUSEADDR option should be enabled to allow a socket to
bind forcibly to a port that is already in use by another socket.
The second socket should invoke setsockopt with the optname
parameter set to SO_REUSEADDR and the optval parameter set to a
boolean value of TRUE before calling bind on the same port as the
original socket.
2. SO_BROADCAST:
• The SO_BROADCAST option should be enabled to allow outgoing
broadcasts from a socket.
3. UDP Socket Binding:
• For UDP sockets that expect to receive messages, it is
recommended to explicitly bind the socket to INADDR_ANY. This
ensures that the application can receive UDP messages on any
available network interface, and it avoids having the operating
system automatically assign a port number.

Some of the concepts we need to incorporate:
Broadcast specific IP like 255.255.255.255
struct socketaddr_in add; addr.sin_addr.s_addr = INADDR_BROADCAST;

set up like this:
doDiscovery
{
SOCKET s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
int optVal = 1;
setsockopt(s, SOL_SOCKET, SO_BROADCAST, $optVal, sizeof(optVal));
sockaddr_in bcAddr;
bcAddr.sin_family = AF_INET;
bcAddr.sin_addr.s_addr = INADDR_BROADCAST;
boardCastAddr.sin_port = htons(svPort);
sendto(s, buffer, length, flags, &bcAddr, sizeof(bcAddr));

The for receive:
SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY
addr.sin_port = htons(svPort)
bind(s,(sockaddr*)&addr,sizeof(addr));
recvfrom(s,buffer,length,flags,NULL,NULL);

*/





// This function will run in a separate thread and will periodically broadcast the server's IP address and TCP port to the local network using UDP.
//it passes a thread-safe flag by reference that controls lifecycle of loop (this is used in place of a mutex)
void ThreadEntryPoint(
	std::atomic<bool>& activeUsers,
	std::string ipAddress,
	uint16_t tcpPort)
{
	//used for timing of broadcast (for steady_clock, miiliseconds and seconds)
	using namespace std::chrono;

	//UDP socket creation 
	//IPv4, SOCK_DGRAM = UDP, UDP protocol
	SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (udpSocket == INVALID_SOCKET)
	{
		std::cout << "UDP socket creation failed\n";
		return;
	}


	// enables broadcast
	BOOL opt = TRUE;
	//SOL_SOCKET = socket-level option, SO_BROADCAST: Enable broadcast permission, OPT point is true
	setsockopt(udpSocket,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));

	// optional reuse
	//allows socket to bind/use address even if another socket recently used it, restarts server quickly, multiple sockets on same port
	setsockopt(udpSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));

	//broadcast address set up (IPv4,port to big endian)
	sockaddr_in broadcastAddr = {};
	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_port = htons(54000);

	// 255.255.255.255 = packets go to all devices on LAN
	broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

	//last time broadcast was sent
	auto lastBroadcast = steady_clock::now();

	std::cout << "UDP Thread Enter\n";

	//while atomic variable is true (becomes false when there are no active current clients/no sockets in use)
	//once escapes loop, will rejoin main thread
	while (activeUsers)
	{
		//start clock, steady clock will not change even if system clock changes
		auto now = steady_clock::now();

		//calculate time between clock and last broadcast
		//duration_cast converts to whole seconds
		auto elapsed = duration_cast<seconds>(now - lastBroadcast);

		//5 second timer
		if (elapsed.count() >= 5)
		{
			//const char* msg = "The server will be offline for routine maintenance tonight starting at 11:59 PM Eastern Time.";

			//build UDP message
			std::string msg = ipAddress + ":" + std::to_string(tcpPort);

			//sends UDP packet to 255.255.255:54000
			//socket that is sending data, paointer to message buffer, size of message in bytes, flags (none), destination address, size of address struct
			int result = sendto(udpSocket,msg.c_str(),(int)msg.size(),0,(sockaddr*)&broadcastAddr,sizeof(broadcastAddr));

			if (result == SOCKET_ERROR)
			{
				//returns windows networking error code
				std::cout << "Broadcast failed: " << WSAGetLastError() << "\n";
			}
			else
			{
				std::cout
					<< "Broadcast sent: " << msg << "\n";
			}

			//once message sent, lastbroadcast becomes now, so difference between now and lastbroadcast is 0 and will grow again toward 5. 
			lastBroadcast = now;
		}

		//limits loop to 20 times per second, so not constantly checking time (since messages only sent every 5 seconds)
		std::this_thread::sleep_for(milliseconds(50));
	}

	//release UDP socket and frees OS resources
	closesocket(udpSocket);

	std::cout << "UDP Thread Exit\n";
}




int Server::InitListeningSocket(uint16_t port, SOCKET& listenSocket, int capacity)
{
	
	//create a socket
	//AF_INET = IPv4, SOCK_STREAM = TCP, IPPROTO_TCP = TCP protocol
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("DEBUG// Socket function incorrect\n");
		return SETUP_ERROR;
	}
	else
	{
		printf("DEBUG// I used the socket function\n");
	}


	//Prepare address struct, sets family to AF_INET (IPv4), address to INADDR_ANY (binds to all available interfaces)
	//passes port in network byte order (htons = host to network short)
	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port); //31337

	//Binds socket to the specified address and port, if it fails, closes the socket and returns an error code
	//Failure might occur if port in use, admin restrictions or invalid address
	//also casts the sockaddr_in struct to a generic sockaddr pointer as required by the bind function, and passes the size of the sockaddr_in struct
	//so that bind knows how much memory to read for the address information
	int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Bind function incorrect\n");
		closesocket(listenSocket);
		return BIND_ERROR;
	}
	else
	{
		printf("DEBUG// I used the bind function\n");
	}

	//Listen
	//turns the socket into a listening socket, allowing it to accept incoming connection requests. 
	// The second parameter (1) specifies the maximum length of the queue of pending connections. 
	// If it fails, closes the socket and returns an error code. 
	// Reasons this could fail include if the socket is not properly bound, if the socket is already listening, 
	// or if there are insufficient system resources to listen on the socket.
	result = listen(listenSocket, capacity);
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Listen function incorrect\n");
		closesocket(listenSocket);
		return CONNECT_ERROR;
	}
	else
	{
		printf("DEBUG// I used the listen function\n");
	}


	return SUCCESS;
}

void Server::DisconnectClient(SOCKET client, fd_set& masterSet, int& currentClients)
{
	Stop(client);

	//select no longer needs to monitor this socket for activity, so it's removed from the master set of sockets
	FD_CLR(client, &masterSet);

	if (currentClients > 0)
	{
		currentClients--;
	}

	printf("Client disconnected\n");
}

void Server::Stop(SOCKET socket)
{
	if (socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}
}

void Server::ServerCode(void)
{
	//UDP Thread
	std::atomic<bool> activeUsers = false;
	std::thread udpThread;


	uint16_t port;
	int capacity;
	char commandChar = '~';

	std::cout << "Port: ";
	std::cin >> port;

	std::cout << "Capacity: ";
	std::cin >> capacity;

	//flush leftover newline
	std::cin.ignore(10000, '\n');

	std::cout << "Command character: ";

	std::string input;
	std::getline(std::cin, input);

	if (!input.empty() && input[0] != ' ' && input[0] != '\t')
	{
		commandChar = input[0];
	}
	else
	{
		std::cout << "Invalid or empty input. Defaulting to '~'\n";
	}


	//declare sockets for functions
	SOCKET listenSocket = INVALID_SOCKET;

	int result = Server::InitListeningSocket(port, listenSocket, capacity);


	//handle errors with listening socket setup, bind and listen
	if (result != SUCCESS)
	{
		switch (result)
		{
		case SETUP_ERROR:
			MessageBoxA(NULL, "Socket setup failed.", "Server Error", MB_OK | MB_ICONERROR);
			break;

		case BIND_ERROR:
			MessageBoxA(NULL, "Bind failed. Port may already be in use.", "Server Error", MB_OK | MB_ICONERROR);
			break;

		case CONNECT_ERROR:
			MessageBoxA(NULL, "Listen failed. Unable to accept connections.", "Server Error", MB_OK | MB_ICONERROR);
			break;

		default:
			MessageBoxA(NULL, "Unknown server error.", "Server Error", MB_OK | MB_ICONERROR);
			break;
		}

		return;
	}

	
	fd_set masterSet;
	fd_set readySet;

	//clear set
	FD_ZERO(&masterSet);
	//adds listening socket to the master set
	FD_SET(listenSocket, &masterSet);

	//initially, the ready set is the same as the master set, but it will be modified by select to indicate which sockets are ready for reading
	readySet = masterSet;

	char host[256];
	gethostname(host, sizeof(host));
	printf("Hostname: %s\n", host);

	addrinfo hints = {};
	addrinfo* addressResult = nullptr;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(host, NULL, &hints, &addressResult);

	std::string serverIP;
	for (addrinfo* ptr = addressResult; ptr != NULL; ptr = ptr->ai_next)
	{
		char ipString[INET6_ADDRSTRLEN];

		void* address;

		if (ptr->ai_family == AF_INET)
		{
			sockaddr_in* ipv4 = (sockaddr_in*)ptr->ai_addr;
			address = &(ipv4->sin_addr);
		}
		else
		{

			sockaddr_in6* ipv6 = (sockaddr_in6*)ptr->ai_addr;
			address = &(ipv6->sin6_addr);
		}

		//make binary IP address human-readable
		inet_ntop(ptr->ai_family, address, ipString, sizeof(ipString));

		//printf("IP Address: %s\n", ipString);

		if (ptr->ai_family == AF_INET)
		{
			printf("IPv4 Address: %s\n", ipString);
			serverIP = ipString;
		}
		else if (ptr->ai_family == AF_INET6)
		{
			printf("IPv6 Address: %s\n", ipString);
		}

	}

	//freeaddrinfo is called to free the memory allocated by getaddrinfo for the linked list of address information
	freeaddrinfo(addressResult);


	int currentClients = 0;
	while (true)
	{

		readySet = masterSet;

		//blocking. waits for activity on any socket in the ready set, or new connections.
		int socketCount = select(0, &readySet, nullptr, nullptr, nullptr);

		SOCKET clientSocket = INVALID_SOCKET;

		//loop through ready sockets and check if activity is on listening socket (new connection) or client socket (incoming message)
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET currentSocket = readySet.fd_array[i];

			if (currentSocket == listenSocket)
			{
				//check for server full, accept then close if server full
				if (currentClients >= capacity)
				{
					SOCKET temp = accept(listenSocket, NULL, NULL);
					if (temp != INVALID_SOCKET)
					{
						std::string msg = "Server full. Please try again later.";
						TCPFraming::sendFrame(temp, msg.c_str(), msg.size());
						printf("Server full - rejecting client\n");
						closesocket(temp);
					}
					continue;
				}

				//accept new connection, add to master set and increment client count
				clientSocket = accept(listenSocket, NULL, NULL);

				if (clientSocket != INVALID_SOCKET)
				{
					FD_SET(clientSocket, &masterSet);
					currentClients++;

					//send welcome message to client
					std::string welcome =
						std::string("Welcome to the chat server. Please use ")
						+ commandChar +
						" before all commands. Use "
						+ commandChar +
						"help for command list.";

					TCPFraming::sendFrame(clientSocket, welcome.c_str(), welcome.size());

					//if current clients is 1, start thread for UDP
					if (currentClients == 1)
					{
						activeUsers = true;
						udpThread = std::thread(
							ThreadEntryPoint,
							std::ref(activeUsers),
							serverIP,
							port);
					
					}

				}
			}
			else
			{
				//read message from client, handle disconnects and shutdowns, pass valid messages to message handler
				char buffer[256] = {};

				int result = TCPFraming::readFrame(currentSocket, buffer, sizeof(buffer));


				if (result == PARAMETER_ERROR)
				{
					printf("Invalid frame received\n");
					continue;
				}


				//Handle graceful shutdowns and connection errors
				if (result == SHUTDOWN || result == DISCONNECT)
				{

					DisconnectClient(currentSocket, masterSet, currentClients);
					AuthManager::logoutUser(currentSocket);

					//make into helper
					//if current clients is 0, can stop UDP thread 
					if (currentClients == 0)
					{
						activeUsers = false;

						if (udpThread.joinable())
						{
							udpThread.join();
						}
					}

					continue;
				}


				bool shouldDisconnect = false;
				shouldDisconnect = MessageHandler::HandleCommand(currentSocket, listenSocket, masterSet, commandChar, buffer, capacity);

				if (shouldDisconnect)
				{
					DisconnectClient(currentSocket, masterSet, currentClients);

					//Make into helper
							//if current clients is 0, can stop UDP thread 
					if (currentClients == 0)
					{
						activeUsers = false;

						if (udpThread.joinable())
						{
							udpThread.join();
						}
					}


				
					continue;
				}

		

			}
		}
	}


}

