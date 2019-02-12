#include <stdio.h>

void finish() {
	for (int i = 10; i >= 0; --i){
		printf("%d * %d = %d\n", i, i, i * i);
	}
}