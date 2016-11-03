#ifndef RCT1_IPC_H
#define RCT1_IPC_H

typedef enum ChannelType {Fifo, Pipe, Socket} ChannelType;
extern const char* const channelNames[];
extern const int nrChannelTypes;

bool openChannel(ChannelType type, int* result);

#endif