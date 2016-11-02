#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>


const char** readCommandChunks(int* nrChunks);
const char** splitArgs(const char** commandChunks, int nrChunks, int* nrArgs);


int clientMain(int inFd, int outFd){
	printf("Connected.\n")

	printf("You should login first (command: login {username})\n");

	int nrChunks;
	char** commandChunks = readCommandChunks(&nrChunks);

	while(commandChunks != NULL){
		int nrArgs;
		char** args = splitArgs(commandChunks, nrChunks, &nrArgs);
		free2d(commandChunks, nrChunks);		

		if (args == NULL){
			perror("splitting arguments from chunks");

			close(inFd);
			close(outFd);
			exit(20);
		}

		sendCommand(args, nrArgs, outFd);
		free2d(args, nrArgs);

		printResponse(inFd);

		commandChunks = readCommandChunks(&nrChunks);
	}
	printf("Client shutting down.");

	close(inFd);
	close(outFd);

	return 0;
}


const char** readCommandChunks(int* nrArgs){
	printf("> ");

	const int chunkSize = 256;
	char** result = NULL;
	*nrArgs = 0;

	bool done = false;

	while(!done){
		char* thisChunk = malloc(chunkSize);
		
		if (fgets(thisChunk, chunkSize, stdin) == NULL){
			free(thisChunk);
			return result;
		}

		int chunkLen = strlen(thisChunk);
		if (thisChunk[chunkLen - 2] == '\r'){
			thisChunk[chunkLen - 2] = '\0';
			done = true;
		}
		else if (thisChunk[chunkLen - 1] == '\n'){
			thisChunk[chunkLen - 1] = '\0';
			done = true;
		}

		if (strlen(thisChunk) == 0 && nrArgs != 0){
			free(thisChunk);
			done = true;
		}
		else{
			nrArgs++;
			realloc(result, sizeof(char*) * nrArgs);
			result[nrArgs - 1] = thisChunk;
		}

	}

	return result;
}


const char* emptyArgs = [ "" ];

const char** splitArgs(const char** commandChunks, int nrChunks, int* nrArgs){
	char** args = NULL:
	if (nrChunks == 1 && strlen(commandChunks[0]) == 0){
		return emptyArgs;
	}

	
}
