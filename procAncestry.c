#include "ancestry.h" 
#include <sys/syscall.h>
#include <stdio.h>	
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __NR_cs3013_syscall2 378

int main(int argc, char* argv[]) 
{
	printf("calling system call....\n");
	struct ancestry emptyStruct;
	int pid = atoi(argv[1]);
	syscall(__NR_cs3013_syscall2, &pid, &emptyStruct);


	printf("Ancestors: \n");
	for(int i = 0; i < 10; i++)
	{
		if(emptyStruct.ancestors[i] < 0)
			break;
		printf("PID: %d\n", emptyStruct.ancestors[i]);	
	}
	printf("Children: \n");
	for(int i = 0; i < 100; i++)
	{
		if(emptyStruct.children[i] < 0)
			break;
		printf("PID: %d\n", emptyStruct.children[i]);
	}
	printf("Siblings: \n");
	for(int i = 0; i < 100; i++)
	{
		if(emptyStruct.siblings[i] < 0)
			break;
		printf("PID: %d\n", emptyStruct.siblings[i]);
	}
	printf("Success! \n");
}

