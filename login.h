#ifndef RCT1_LOGIN_H
#define RCT1_LOGIN_H

bool checkLogin(const char* username, const char* password, UserRights* rights);
bool register(const char* username, const char* password, UserRights rights);

bool usernameExists(const char* username);

#endif