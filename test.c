#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char * args[]) 
{
	printf("Running the system call 1... \n");
	int rc;
	rc = syscall(377);
	if(rc == -1) 
	{
		printf("Call failed...\n");
	}
}
