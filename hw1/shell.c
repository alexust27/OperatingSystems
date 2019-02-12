#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	while(1) {
		printf("~ $ ");
		char* s;
		char next;
		int i = 0;
		size_t size = 32;
		s = (char*)malloc(size);
		char* args[32];
		int i_arg = 0;
		while((next = getchar()) != EOF){
			if (i > size)
			{
				size *= 2;
				s = (char*)realloc(s, size);
			}
			s[i] = next;

			while (next == ' ' || next == '\t' || next == '\r') {
				next = getchar();
			}
			
			if (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n') {
				if (i > 0) {
					args[i_arg] = (char*)malloc(i);
					strncpy(args[i_arg], s, i);
					i_arg++;
				}
				free(s);
				s = (char*)malloc(size);
				i = 0;
				s[i] = next;
			}
			if (next == '\n'){
				break;
			}
			i++;
		}
		if(next == EOF){
			printf("\n");
			break;
		}
		args[i_arg] = NULL;
		if (i_arg == 0)
			continue;
		
		if (!strcmp(args[0], "exit")){
			exit(0);
		}
		
		pid_t pid = fork();
		int ex = -1;
		if (pid < 0) {
			perror("fork failed");
			exit(1);
		}
		else if (pid == 0) {
			execvp(args[0], args);
			perror("execve");
			exit(errno);
		}
		int status;
		waitpid(pid, &status, 0);
		printf("exit code: %d\n",(short)status);
	}

	return 0;
}