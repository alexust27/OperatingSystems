#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

char* get_string();
int* make_arr(int n);

int main(int argc, char const *argv[])
{
	printf("%s\n", get_string());

	int *x = make_arr(10);
	printf("1st dynamic lib\n");
	for(int i = 0; i < 10; ++i){
		printf("%d ", x[i]);
	}
	printf("\n");
	printf("2nd dynamic lib\n");
	
	void * handle;
	char * (*func)();
	handle = dlopen("d_lib2.so", RTLD_LAZY);
	if (!handle){
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	};
	func = dlsym(handle, "finish");
	(*func)();

	dlclose(handle);
	exit(EXIT_SUCCESS);
}