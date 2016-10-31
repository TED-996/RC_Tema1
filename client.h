#ifndef RCT1_CLIENT_H
#define RCT1_CLIENT_H

typedef enum {RightRegister = 1, RightProtCmd = 2, RightLtdCmd = 4, RightSudo = 8} UserRights;

void clientMain(int inFd, int outFd);

#endif