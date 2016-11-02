#ifndef RCT1_IPC_H
#define RCT1_IPC_H

typedef enum ChannelType {Fifo, Pipe, Socket} ChannelType;
const char* const channelNames {"fifo", "pipe", "socket"};

bool openChannel(ChannelType type, int* result);

#endif