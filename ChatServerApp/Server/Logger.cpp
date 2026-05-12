#include "Logger.h"


std::string Logger::GetTimestamp()
{
	// Get current time
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	// Format time (YYYY-MM-DD HH:MM)
	std::tm tm_struct;

#ifdef _WIN32
	localtime_s(&tm_struct, &now_time); // Windows safe version
#else
	localtime_r(&now_time, &tm_struct); // Linux/macOS
#endif

	std::ostringstream timeStream;
	timeStream << std::put_time(&tm_struct, "%Y-%m-%d %H:%M");
	std::string timestamp = timeStream.str();

	return timestamp;

}

std::string Logger::GetCurrentUser(SOCKET client)
{
	std::string currentUser = "Unknown User";
	auto it = AuthManager::loggedInUsers.find(client);
	if (it != AuthManager::loggedInUsers.end())
	{
		currentUser = it->second;
	}
	return currentUser;
}

void Logger::LogCommand(SOCKET client, const std::string& commandStr)
{

	std::string timestamp = GetTimestamp();

	//search for username associated with client socket, and log that username with the message, so can track who is sending which messages in server logs
	//if client not in loggedInUsers map for some reason, will log as "Unknown User"
	std::string currentUser = Logger::GetCurrentUser(client);

	std::istringstream iss(commandStr);

	std::string command;
	std::string name; 
	std::string password;

	iss >> command >> name >> password;

	if (!password.empty())
	{
		password = "*********"; // mask password in logs for security
	}


	std::string encryptMsg = std::string(command + " " + name + " " + password); // reconstruct message for logging, with masked password

    std::ofstream file("./Logs/command_log.txt", std::ios::app);

    if (file.is_open())
    {
        file << "[" << timestamp << "] [" << currentUser << "] " << encryptMsg << std::endl;
    }
    else
    {
        std::cout << "Could not open log file\n";
    }
}

void Logger::LogPublicMessage(SOCKET client, const std::string& message)
{
	std::string timestamp = GetTimestamp();
	std::string currentUser = Logger::GetCurrentUser(client);

	std::ofstream file("./Logs/public_chat_log.txt", std::ios::app);

	if (file.is_open())
	{
		file << "[" << timestamp << "] [" << currentUser << "] " << message << std::endl;

		//all public messages printed to server. 
		printf("Public message: [%s] %s\n", currentUser.c_str(), message.c_str());
	}
	else
	{
		std::cout << "Could not open log file\n";
	}
}

std::string Logger::GetPublicChatLog()
{
	/*
	//This is blocking so took out
	std::string filename = "./Logs/public_chat_log.txt";
	std::string command = "notepad \"" + filename + "\"";
	std::system(command.c_str());
	*/

	//opens chat log as a popup in notepad
	std::string filename = "./Logs/public_chat_log.txt";
	ShellExecuteA(NULL, "open", "notepad.exe", filename.c_str(), NULL, SW_SHOW);

	//used to print the chat log to the chat GUI
	std::ifstream file("./Logs/public_chat_log.txt");
	std::stringstream buffer;
	if (file.is_open())
	{
		buffer << file.rdbuf();
		file.close();
	}
	else
	{
		std::cout << "Could not open log file\n";
	}
	return buffer.str();
}