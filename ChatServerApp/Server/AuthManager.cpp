#include "AuthManager.h"


namespace AuthManager
{
    std::unordered_map<std::string, std::string> users;
    std::unordered_map<SOCKET, std::string> loggedInUsers;

}

void AuthManager::AddUser(const std::string& username, const std::string& password)
{
    AuthManager::users[username] = password;
}

bool AuthManager::UserExists(const std::string& username)
{
    return AuthManager::users.find(username) != users.end();
}

bool AuthManager::CheckPassword(const std::string& username, const std::string& password)
{
    auto it = AuthManager::users.find(username);
    if (it == users.end())
        return false;
	return it->second == password;
}


void AuthManager::SetLoggedIn(SOCKET client, const std::string& username)
{
    AuthManager::loggedInUsers[client] = username;
}


bool AuthManager::IsLoggedIn(SOCKET client)
{
    return AuthManager::loggedInUsers.find(client) != AuthManager::loggedInUsers.end();
}

bool AuthManager::IsUserLoggedInAnywhere(const std::string& username)
{
    for (const auto& pair : loggedInUsers)
    {
        if (pair.second == username)
            return true;
    }
    return false;
}


 AuthManager::RegisterResult AuthManager::registerUser(const std::string& username, const std::string& password)
 {
    // 1. check if username exists
        if (users.find(username) != users.end())
        {
            return RegisterResult::USER_TAKEN;
        }

        // 2. validate password length
        if (password.length() < 5)
        {
            return RegisterResult::PASSWORD_INVALID;
        }


        // 3. store user
       //users[username] = password;
        AuthManager::AddUser(username, password);       

        return RegisterResult::SUCCESS;
  }

