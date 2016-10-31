#ifndef RCT1_LOGIN_H
#define RCT1_LOGIN_H

bool checkLogin(char* username, char* password, UserRights* rights);
bool register(char* username, char* password, UserRights rights);

#endif