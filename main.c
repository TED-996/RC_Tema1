#include <stdio.h>
#include <strings.h>

#include "ipc.h"
#include "util.h"


int main(int argc, char* argv){
	if (argc != 2){
		fprintf(stderr, "Bad arguments.\nUsage: RCT1 {pipe|fifo|socket}\n");
		exit(1);
	}
	
	ChannelType channelType;
	for (channelType = 0; channelType < NrElements(fdNames); fdType++){
		if (strcasecmp(argv[1], channelNames[(int)fdType]) == 0){
			break;
		}
	}

	if (type >= NrElements(channelNames)){
		fprintf(stderr, "Unrecognized IPC type. Choose either pipe, fifo or socket.");
		exit(1);
	}

	printf("Establishing IPC channels...");

	int commandChannel[2];
	if (!openChannels(channelType, commandChannel)){
		perror("opening command channel");
		exit(2);
	}

	int responseChannel[2];
	if (!openChannel(channelType, responseChannel)){
		perror("opening response channel");
		exit(2);
	}

	printf("Booting up server...\n");
	int serverPid = spawn2Channel(commandChannel, responseChannel);

	int outFd = commandChannel[1];
	int inFd = responseChannel[0];

	runClient(inFd, outFd);
}
