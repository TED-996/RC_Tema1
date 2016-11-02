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
			exit(30);
		}

		if (!sendCommand(args, nrArgs, outFd)){
			perror("sending command to server");

			close(inFd);
			close(outFd);
			exit(21);
		}

		free2d(args, nrArgs);

		printResponse(inFd);

		commandChunks = readCommandChunks(&nrChunks);
	}
	printf("Client shutting down.");

	close(inFd);
	close(outFd);

	return 0;
}


const char** readCommandChunks(int* nrChunks){
	printf("> ");

	const int chunkSize = 256;
	char** result = NULL;
	*nrChunks = 0;

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

		if (strlen(thisChunk) == 0 && *nrChunks != 0){
			free(thisChunk);
			done = true;
		}
		else{
			*nrChunks++;
			result = realloc(result, sizeof(char*) * *nrChunks);
			result[*nrChunks - 1] = thisChunk;
		}

	}

	return result;
}


bool isWhitespace(char ch);
char* chrNext(const char** chunks, char* ptr, int* chunkIdx);
char* newChunkedSubstr(const char** chunks, char* startPtr, int startChunk, char* endPtr, int endChunk);

const char** splitArgs(const char** commandChunks, int nrChunks, int* nrArgs){
	char** args = NULL:
	if (nrChunks == 1 && strlen(commandChunks[0]) == 0){
		args = malloc(sizeof(char*));
		args[0] = malloc(sizeof(char));
		args[0][0] = '\0';

		return args;
	}

	int chunkIdx = 0;
	char* chrPtr = commandChunks[0];
	
	bool inWord = 0;
	char* argStart;
	int chunkStart;
	char separator = '\0';
	while (chunkIdx != nrChunks || *chrPtr != '\0'){
		if (!inWord){
			if (!isWhitespace(*chrPtr)){
				inWord = true;

				if (*chrPtr == '"' || *chrPtr == '\''){
					separator = *chrPtr;
					chunkStart = chunkIdx;
					argStart = chrNext(commandChunks, chrPtr, &chunkStart);
				}
				else{
					separator = '\0';
					chunkStart = chunkIdx;
					argStart = chrPtr;
				}	
			}
		}
		else{
			if ((separator != '\0' && *chrPtr == separator) || (separator == '\0' && isWhitespace(chrPtr))){
				char* newArg = newChunkedSubstr(commandChunks, argStart, chunkStart, chrPtr, chunkIdx);
				if (newArg == NULL){
					perror("splitting command into arguments");
					
					free2d(args, nrArgs);
					return NULL;
				}
				
				*nrArgs++;
				args = realloc(args, sizeof(char*) * *nrArgs);
				args[*nrArgs - 1] = newArg;
			}
		}

		chrPtr = chrNext(commandChunks, chrPtr, &chunkStart);
	}
}

char* chrNext(const char** chunks, char* ptr, int* chunkIdx){
	ptr++;
	if (*ptr == '\0'){
		*chunkIdx++;
		return chunks[chunkIdx];
	}
	return ptr;
}

bool isWhitespace(char ch){
	return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\n' || ch == '\r';
}

char* newChunkedSubstr(const char** chunks, char* startPtr, int startChunk, char* endPtr, int endChunk){
	result = NULL;
	if (startChunk == endChunk){
		if (endPtr < startPtr){
			return NULL;
		}

		result = malloc(endPtr - startPtr + 1);
		strncpy(result, startPtr, endPtr - startPtr);
		result[endPtr - startPtr] = '\0';

		return result;
	}

	int length = strlen(startPtr);
	int chunkIdx = startChunk + 1;

	while(chunkIdx != endChunk){
		length += strlen(chunks[chunkIdx]);
	}
	length += endPtr - chunks[endChunk];

	result = malloc(length + 1);
	if (result == NULL){
		return NULL;
	}

	strcpy(result, startPtr);
	chunkIdx = startChunk + 1;

	while(chunkIdx != endChunk){
		strcat(result, chunks[chunkIdx]);
	}
	strncat(result, chunks[endChunk], endPtr - chunks[endChunk]);
	result[length] = '\0';

	return result;

}
