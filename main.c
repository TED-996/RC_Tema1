#include <stdio.h>
#include <strings.h>
#include <sys/wait.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "ipc.h"
#include "util.h"
#include "client.h"
#include "login.h"


bool createRootUser();


int main(int argc, char* argv[]){
	if (argc != 2){
		fprintf(stderr, "Bad arguments.\nUsage: RCT1 {pipe|fifo|socket}\n");
		exit(1);
	}

	if (!usernameExists("root")){
		if (!createRootUser()){
			perror("creating root user");
			exit(10);
		}
	}
	
	ChannelType channelType;
	for (channelType = 0; channelType < nrChannelTypes; channelType++){
		if (strcasecmp(argv[1], channelNames[(int)channelType]) == 0){
			break;
		}
	}

	if (channelType >= nrChannelTypes){
		fprintf(stderr, "Unrecognized IPC type. Choose either pipe, fifo or socket.");
		exit(1);
	}

	printf("Establishing IPC channels...");

	int commandChannel[2];
	if (!openChannel(channelType, commandChannel)){
		perror("opening command channel");
		exit(2);
	}

	int responseChannel[2];
	if (!openChannel(channelType, responseChannel)){
		perror("opening response channel");
		exit(2);
	}

	printf("Booting up server...\n");
	int serverPid = spawnSplitChannels(serverMain, commandChannel, responseChannel);
	if (serverPid == -1){
		perror("booting up server");
		exit(11);
	}

	int outFd = commandChannel[1];
	int inFd = responseChannel[0];

	int retcode = clientMain(inFd, outFd);

	printf("Waiting to close server...\n");

	if (wait(NULL) == -1){
		perror("waiting for server process to shut down...");
		exit(11);
	}

	printf("Done.\n");

	return retcode;
}


bool getPassword(char* dest, int maxLen);

bool createRootUser(){
	if (usernameExists("root")){
		fprintf(stderr, "Root user already exists.");
		return false;
	}

	printf("There is no root user, you have to create one.\n");

	printf("Username: root\n");

	char password1[256];
	char password2[256];

	while(true){
		printf("Password: ");
		while (!getPassword(password1, 256)){
			printf("\nPassword too long, try again: ");
		}

		printf("\nOK, write again for verification: ");
		while (!getPassword(password2, 256)){
			printf("\nPassword too long, try again: ");
		}

		if (strcmp(password1, password2) == 0){
			break;
		}
		printf("\nThe passwords do not match, try again.\n");
	}

	printf("\nRegistering...\n");
	if (!registerUser("root", password1, (UserRights)0xFF)){
		perror("registering root user");
		exit(16);
	}
	
	printf("User created\n");

	return true;
}

char getchHidden(){
	//http://stackoverflow.com/a/6856689

	struct termios oldTerm;
	if (tcgetattr(0, &oldTerm) != 0){
		perror("getting terminal options for getch");
		exit(15);
	}
	
	struct termios newTerm;
	newTerm = oldTerm;
	newTerm.c_lflag &= ~(ICANON | ECHO);
	if (tcsetattr(0, TCSAFLUSH, &newTerm) != 0){
		perror("setting new terminal options for getch");
		exit(15);
	}

	char result = getchar();

	if (tcsetattr(0, TCSAFLUSH, &oldTerm) != 0){
		perror("setting original terminal options for getch");
		exit(15);
	}

	return result;
}

bool getPassword(char* dest, int maxLen){
	char* ptr = dest;
	*ptr = getchHidden();
	while(*ptr != '\n' && ptr - dest < maxLen - 1){
		ptr++;
		*ptr = getchHidden();
	}

	if (*ptr == '\n'){
		*ptr = '\0';
		return true;
	}

	return false;
}
