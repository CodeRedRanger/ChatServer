#pragma once
#include <string>

struct User 
{
    std::string username;
    std::string password;
    bool registered = false;
    bool loggedIn = false;
};