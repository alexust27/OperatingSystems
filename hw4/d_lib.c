#include <stdlib.h>

int* make_arr(int n) {
	int* a = malloc(n*sizeof(int));
	for (int i = 0; i < n; ++i){
		a[i] = i;
	}
	return a;
}