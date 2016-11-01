#ifndef RCT1_IPC_H
#define RCT1_IPC_H

typedef enum ChannelType {FIFO, PIPE, SOCKET} ChannelType;
const char* const channelNames {"fifo", "pipe", "socket"};

bool openChannel(ChannelType type, int* result);

#endif