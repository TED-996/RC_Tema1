#ifndef RCT1_SERVER_H
#define RCT1_SERVER_H

typedef enum {RightRegister = 1, RightProtCmd = 2, RightLtdCmd = 4, RightSudo = 8} UserRights;

int serverMain(int inFd, int outFd);

#endif