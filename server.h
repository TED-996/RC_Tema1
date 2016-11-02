#ifndef RCT1_SERVER_H
#define RCT1_SERVER_H

typedef enum {RightRegister = 1, RightLtdCmd = 2, RightSysCmd = 4, RightSudo = 8} UserRights;

int serverMain(int inFd, int outFd);

#endif