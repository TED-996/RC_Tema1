#include<stdio.h>
#include<unistd.h>
#include<dirent.h>
#include<linux/limits.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<sys/types.h>


#include "base.h"
#include "myfuncs.h"
#include "util.h"


int walk(char* path, const char* pattern, int logFd);

void myFind(const char* basePath, const char* data, int outFd){
	char pathBuffer[PATH_MAX];
	strcpy(pathBuffer, basePath);
	walk(pathBuffer, data, outFd);
}

bool writeStatFor(const char* path, int outFd);

void myStat(const char* path, int outFd){
	writeStatFor(path, outFd);
}


int writeFound(const char* foundPath, int logFd);
bool is_dir(const char* path);
bool isFakeEntry(const char* entry);

int walk(char* path, const char* pattern, int logFd){
    char* dir_end = &path[strlen(path)];
    *dir_end = '/';
    char* dir_start = dir_end + 1;
    *dir_start = '\0';
    int result = 0;

    DIR* dir = opendir(path);
    if (dir == NULL){
        writeSizedStr(logFd, "Directory ");
		writeSizedStr(logFd, path);
		writeSizedStr(logFd, " inaccessible.\n");
        return 0;
    }
    struct dirent* ent = readdir(dir);
    while (ent != NULL){
        if (isFakeEntry(ent->d_name)){
            ent = readdir(dir);
            continue;
        }

        strcpy(dir_start, ent->d_name);
        if (strstr(ent->d_name, pattern)){
            if (!writeFound(path, logFd)){
                perror("myfind: writing found location");

                if (closedir(dir) != 0){
                    perror("myfind: closing directory");
					exit(42);
                }
                *dir_end = '\0';
                exit(41);
            }
            result++;
        }

        if (is_dir(path)){
            result += walk(path, pattern, logFd);
        }
        ent = readdir(dir);
    }
    if (closedir(dir) != 0){
        perror("closing directory");
		exit(42);
    }
    *dir_end = '\0';
    return result;
}


bool is_dir(const char* path){
    struct stat st;
    if (stat(path, &st) == -1){
        return 0;
    }

    return S_ISDIR(st.st_mode);
}


int writeFound(const char* foundPath, int logFd){
    char buf[PATH_MAX];
    int bufLen = sprintf(buf, "Found: %s\n", foundPath);

    if (writeSizedStr(logFd, buf) != bufLen){
        perror("writing results");
		exit(41);
    }

	if (!writeStatFor(foundPath, logFd)){
		perror("writing stat for found file");
		exit(43);
	}

    return 1;
}

bool isFakeEntry(const char* entry){
    return entry[0] == '.' && (entry[1] == '\0' || (entry[1] == '.' && entry[2] == '\0'));
}

bool writeStatFor(const char* path, int logFd){
	struct stat st;
    if (stat(path, &st) == -1){
		writeSizedStr(logFd, "File ");
		writeSizedStr(logFd, path);
		writeSizedStr(logFd, " inaccessible.\n");
        return false;
    }

	writeSizedStr(logFd, "Stat for ");
	writeSizedStr(logFd, path);
	writeSizedStr(logFd, ":\n");

	char dataBuffer[4096];
	int dataLen = sprintf(dataBuffer, "Size: %d\tDevice: %x\tInode: %d\nUid: %d\tGid: %d\tAccess: %x\n",
                            (int)st.st_size, (int)st.st_dev, (int)st.st_ino, (int)st.st_uid, (int)st.st_gid, (int)st.st_mode);
	if (writeSizedStr(logFd, dataBuffer) != dataLen){
		perror("writing stat lines 1");
		exit(44);
	}

	dataLen = sprintf(dataBuffer, "Access: %d s\tModif: %d s\tChange: %d s (Unix)\n",
                            (int)st.st_atime, (int)st.st_mtime, (int)st.st_ctime);

	if (writeSizedStr(logFd, dataBuffer) != dataLen){
		perror("writing stat lines 2");
		exit(44);
	}

	return true;
}

