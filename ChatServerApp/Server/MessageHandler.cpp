#include "MessageHandler.h"

void MessageHandler::HandleLogin(SOCKET client, const std::string& msg)
{

	//split income string into tokens by whitespace, first token is command, second is username, third is password, if missing fields, will be handled by AuthManager
	std::istringstream iss(msg);

	std::string command;
	std::string username;
	std::string password;

	iss >> command >> username >> password;

	//delegates logging user to AuthManager, and results are handled below.
	auto result = AuthManager::loginUser(username, password, client);

	if (result == AuthManager::LoginResult::MISSING_FIELDS)
	{
		std::string missingFields = "Missing fields. Please provide both username and password.";
		TCPFraming::sendFrame(client, missingFields.c_str(), missingFields.size());
		return;
	}

	if (result == AuthManager::LoginResult::USER_DOES_NOT_EXIST)
	{
		std::string noUser = "User does not exist.";
		TCPFraming::sendFrame(client, noUser.c_str(), noUser.size());
		return;
	}

	if (result == AuthManager::LoginResult::PASSWORD_INVALID)
	{
		std::string invalidPassword = "Invalid password.";
		TCPFraming::sendFrame(client, invalidPassword.c_str(), invalidPassword.size());
		return;
	}

	if (result == AuthManager::LoginResult::ALREADY_LOGGEDIN)
	{
		std::string alreadyLoggedIn = "That user is already logged in.";
		TCPFraming::sendFrame(client, alreadyLoggedIn.c_str(), alreadyLoggedIn.size());
		return;
	}
	
	// SUCCESS
	if (result == AuthManager::LoginResult::SUCCESS)
	{
		std::string successMsg = "Login successful";
		TCPFraming::sendFrame(client, successMsg.c_str(), successMsg.size());
	}
	
}


void MessageHandler::HandleRegister(SOCKET client, const std::string& msg)
{
	//split income string into tokens by whitespace, first token is command, second is username, third is password, if missing fields, will be handled by AuthManager
	std::istringstream iss(msg);

	std::string command;
	std::string username;
	std::string password;

	iss >> command >> username >> password;

	//delegates registering user to AuthManager, and results are handled below.
	auto result = AuthManager::registerUser(username, password);


	switch (result)
	{
	case AuthManager::RegisterResult::SUCCESS:
	{

		//create wrapper for below so code doesn't have to be repeated below x 2
		std::string msg = "Username registration succeeded. Please log in."; 
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}

	case AuthManager::RegisterResult::MISSING_FIELDS:
	{

		//create wrapper for below so code doesn't have to be repeated below x 2
		std::string msg = "Missing fields. Please provide both username and password.";
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}

	case AuthManager::RegisterResult::USER_TAKEN:
	{
		std::string msg = "Username taken.";
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}
	case AuthManager::RegisterResult::PASSWORD_INVALID:
	{
		std::string msg = "Password invalid (must be at least 5 characters)";
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}
	}
}

//handles get help command by sending a list of available commands to the client, using the provided command character to show how to use each command
void MessageHandler::HandleGetHelp(SOCKET client, const char cmdChar)
{
	std::string commandList = "To register: ";
	commandList += std::string(1, cmdChar) + "register username password\n";
	commandList += "To login: ";
	commandList += std::string(1, cmdChar) + "login username password\n";
	commandList += "To get help: ";
	commandList += std::string(1, cmdChar) + "help\n";
	TCPFraming::sendFrame(client, commandList.c_str(), (uint16_t)commandList.size());
}

//routing brain
void MessageHandler::HandleCommand(SOCKET client, SOCKET listenSocket, fd_set& masterSet, const char cmdChar, const char* msg)
{

	//makes sure client is logged in before anything messages can be sent. If not logged in, does allow for register, login and help commands.
	if (!AuthManager::IsLoggedIn(client))
	{
		std::string cmd(msg);
		//cleans up any line endings so "bool allowed" works appropriately.
		cmd.erase(std::remove(cmd.begin(), cmd.end(), '\r'), cmd.end());
		cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n'), cmd.end());

		bool allowed =
			cmd.rfind(std::string(1, cmdChar) + "register", 0) == 0 ||
			cmd.rfind(std::string(1, cmdChar) + "login", 0) == 0 ||
			cmd.rfind(std::string(1, cmdChar) + "help", 0) == 0;

		//if the command is not one of the allowed commands for non-logged-in users, send a message to the client and return without processing the command further 
		// essentially blocking any other commands until the user logs in or registers, but allows client to ask for help
		if (!allowed)
		{
			std::string mustLogin =
				"You must register or login first. Use "
				+ std::string(1, cmdChar)
				+ "register or "
				+ std::string(1, cmdChar)
				+ "login.";

			TCPFraming::sendFrame(client, mustLogin.c_str(), mustLogin.size());
			return; 
		}
	}
	
	//for non-commands. 
	//right now every message from client goes to all other logged-in clients
	//eventually will be able to do private messages by putting $username at the beginning of message, and only sending to that user, but for now, this is fine.
	if (!msg || msg[0] != cmdChar)
	{

		//for each socket
		for (int i = 0; i < masterSet.fd_count; i++)
		{
			SOCKET s = masterSet.fd_array[i];

			//if not listening socket, and not the client who sent the message
			if (s == listenSocket || s == client)
				continue;

			//if not logged in, skip sending message to that socket, effectively blocking non-logged-in users from receiving chat messages
			if (!AuthManager::IsLoggedIn(s))
				continue; // block non-logged-in users from receiving chat

			//otherwise can send message to socket s
			TCPFraming::sendFrame(s, msg, (uint16_t)strlen(msg));
		
		}


		//HERE HERE HERE
		//do I need below and should I move print to broadcast message function? 

		/*
			broadcastMessage(msg);
		*/

		//all messages printed to server. 
		//eventually will log all public messages so can print out complete chat history (need additional command)
		printf("Public message: %s\n", msg);
		return;
	}

	//handle commands (can expand later to include logout, private message, etc.)
	std::string registerCmd = std::string(1, cmdChar) + "register";
	std::string loginCmd = std::string(1, cmdChar) + "login";
	std::string helpCmd = std::string(1, cmdChar) + "help";
	std::string logoutCmd = std::string(1, cmdChar) + "logout";

	//checks if the beginning of the message matches any of the known commands, and if so, calls the appropriate handler function for that command. 
	if (strncmp(msg, registerCmd.c_str(), registerCmd.length()) == 0)
	{
		printf("REGISTER COMMAND RECEIVED\n");
		MessageHandler::HandleRegister(client, msg);
	}
	else if (strncmp(msg, loginCmd.c_str(), loginCmd.length()) == 0)
	{
		printf("LOGIN COMMAND RECEIVED\n");
		MessageHandler::HandleLogin(client, msg);
	}
	else if (strncmp(msg, helpCmd.c_str(), helpCmd.length()) == 0)
	{
		printf("GETHELP COMMAND\n");
		MessageHandler::HandleGetHelp(client, cmdChar);
	}
	//handle logout too
	/*
	
	else if (strncmp(msg, logoutCmd.c_str(), logoutCmd.length()) == 0)
	{
		printf("LOGOUT COMMAND RECEIVED\n");
		MessageHandler::HandleLogout(client, cmdChar);
		//model after handle login above and improve login function in AuthManager
	}

	*/
	else
	{
		printf("UNKNOWN COMMAND\n");
	}
}




