#ifndef RCT1_LOGIN_H
#define RCT1_LOGIN_H

#include "base.h"
#include "server.h"

bool checkLogin(const char* username, const char* password, UserRights* rights);
bool registerUser(const char* username, const char* password, UserRights rights);

bool usernameExists(const char* username);

#endif