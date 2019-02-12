#include <stdio.h>
#include <sys/types.h>
#include <unistd.h> 

int main(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		printf("%s\n", argv[i]);
	}
	for (int i = 0; i < 100000; ++i)
	{
		/* code */
	}
	sleep(1);
	printf("Hello world\n");
	printf("pid: %d\n",(int) getpid() );
	return 0;
}