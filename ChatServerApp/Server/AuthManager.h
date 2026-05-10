#pragma once
#include <string>
#include <unordered_map>
#include <ws2tcpip.h>
#include "../User.h"


namespace AuthManager
{

    enum class RegisterResult
    {
        SUCCESS,
        USER_TAKEN,
        PASSWORD_INVALID
    };

    RegisterResult registerUser(const std::string& username, const std::string& password);
    void AddUser(const std::string& username, const std::string& password);
    bool UserExists(const std::string& username); 

    bool CheckPassword(const std::string& username, const std::string& password);
    void SetLoggedIn(SOCKET client, const std::string& username);
    bool IsLoggedIn(SOCKET client);
    bool IsUserLoggedInAnywhere(const std::string& username); 

    extern std::unordered_map<std::string, std::string> users;
    extern std::unordered_map<SOCKET, std::string> loggedInUsers;
    extern size_t maxUsers;

};

