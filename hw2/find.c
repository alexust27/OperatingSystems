#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

struct  {
	ino_t inum;
	nlink_t nlink;
	off_t size;
	int cmpSize;
	char exec[1024];
	char name[256];
};

void init(){
	pattern.inum = -1;
	pattern._nlink = -1;
	pattern.size = -1;
	pattern.cmpSize = 0;
	pattern.name[0] = '.';
	pattern.name[1] = '\0';
	pattern.exec[0] = '\0';
}

int cmpSize(off_t size){
	if (pattern.size == size){
		return 0;
	}
	if (pattern.size > size) {
		return -1;
	}
	return 1;
}


void exec(char pathName[256]){
	if (strcmp(pattern.exec, "") == 0) {
		printf("%s\n", pathName);
		return;
	}
	const pid_t pid = fork();
	char * tokens[3] = { pattern.exec, pathName, NULL };

	int status = 0;
	int ex = -1;
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	}
	else if (pid == 0) {
		execvp(pattern.exec, tokens);
		perror("execve");
		exit(errno);
	}
	waitpid(pid, &status, 0);
}

void check(char *name, ino_t inum, nlink_t nlink, off_t size, char pathName[256]) {
	if (strcmp(pattern.name, ".") != 0 && strcmp(pattern.name, name) != 0) {
		return;
	}
	if (pattern.inum != -1 && inum != pattern.inum) {
		return;
	}

	if (pattern.nlink != -1 && nlink != pattern.nlink) {
		return;
	}
	if (pattern.size != -1 && cmpSize(size) != pattern.cmpSize) {
		return;
	}
	exec(pathName);
}

void visiting(char *nameDir){

	struct dirent *entry = NULL;
	DIR* dir = opendir(nameDir);
	char pathName[256];
	if (dir == NULL) {
		fprintf(stderr, "Error opening directory on path %s: %s\n", nameDir, strerror(errno));
		return;
	}

	while ((entry = readdir(dir))) {
		struct stat info;
		if (strcmp(entry->dname, ".") == 0 ||
			strcmp(entry->dname, "..") == 0){
			continue;
		}
		strcpy(pathName, nameDir);
		strcat(pathName, "/");
		strcat(pathName, entry->d_name);
		if (stat(pathName, &info) == 0) {
			if (S_ISDIR(info.st_mode)) {
				visiting(pathName);
			}
			else if (S_ISREG(info.st_mode)) {
				check(entry->d_name, info.st_ino,
					info.st_nlink, info.stsize, pathName);
			}
		}
		else {

			fprintf(stderr, "Can't get stat file with name %s in directory on the path %s: %s\n",
				entry->d_name, nameDir, strerror(errno));
		}

	}
	closedir(dir);

}

void createPattern(int argv, char * argc[]) {
	for (int i = 2; i < argv; i += 2){
		if (i + 1 == argv) {
			printf("Invalid arguments.");
			exit(EXIT_FAILURE);
		}
		if (strcmp("-inum", argc[i]) == 0) {
			pattern.inum = atoi(argc[i + 1]);

		}
		else if (strcmp("-name", argc[i]) == 0) {
			strcpy(pattern._name, argc[i + 1]);
		}
		else if (strcmp("-size", argc[i]) == 0) {
			int k = 0;
			switch (argc[i + 1][0]){
			case '-': pattern.cmpSize = -1; k = 1; break;
			case '+': pattern.cmpSize = 1; k = 1; break;
			case '=': pattern.cmpSize = 0; k = 1; break;
			}
			pattern.size = atoi(argc[i + 1] + k);

		}
		else if (strcmp("-nlinks", argc[i]) == 0) {
			pattern._nlink = atoi(argc[i + 1]);

		}
		else if (strcmp("-exec", argc[i]) == 0) {
			strcpy(pattern.exec, argc[i + 1]);
		}
		else {
			printf("Unexpected token or Invalid arguments - { %s }", argc[i]);
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argN, char * args[]) {
	if (argN < 2) {
		printf("Expected more arguments.");
		return EXIT_SUCCESS;
	}
	init();
	createPattern(argN, args);
	visiting(args[1]);
	return EXIT_SUCCESS;
}

