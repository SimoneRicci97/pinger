#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) {
	printf("Hello father!\n");
	printf("I was born with argc=%d\n", argc);
	for(int i=0; i<argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}
	return 1;
}