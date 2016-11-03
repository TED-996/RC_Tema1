#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "base.h"
#include "ipc.h"

const char* const channelNames[] = {"fifo", "pipe", "socket"};
const int nrChannelTypes = 3;

int getFifoName(char* buffer);


bool openChannel(ChannelType type, int* result){
	if (type == Fifo){
		char name[300];
		getFifoName(name);
		if (mkfifo(name, 0666) == 0){
			perror("creating FIFO");
			return false;
		}

		result[0] = open(name, O_RDONLY | O_NONBLOCK);
		
		if (result[0] == -1){
			perror("opening FIFO read end");
			return false;
		}

		result[1] = open(name, O_WRONLY);
		
		if (result[1] == -1){
			perror("opening FIFO write end");
			return false;
		}

		int flags = fcntl(result[0], F_GETFD);
		fcntl(result[0], F_SETFD, flags & (~O_NONBLOCK));
		
		return true;		
	}
	else if (type == Pipe){
		if (pipe(result) != 0){
			return false;
		}
		return true;
	}
	else if (type == Socket){
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, result) != 0){
			return false;
		}
		return true;
	}

	return false;
}

int fifoUid = 0;

int getFifoName(char* buffer){
	return sprintf(buffer,  "%s/%s_p%d_%d.%s", "~/dev/fifo", "tema1", getpid(), fifoUid++, "fifo");
}
