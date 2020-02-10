#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char * args[]) 
{
	printf("Running the system call 1... \n");
	int rc;
	rc = syscall(377);
	if(rc == -1) 
	{
		printf("Call failed...\n");
	}

	char * buff = "12345zoinks!sksjdlaksjd";
	char * token = "zoinks!";
	int malicious = 0;
	int count = strlen(buff);
	if( count >= 7 ) 
	{
		for(int i = 0; i < count; i++) 
		{
			//check against token.
			char current = buff[i];
			if (current == 'z')
			{
				//don't want to go out of the bounds of the buffer
				if(i + 7 <= count) 
				{
					for(int j = 0; j < 7; j++)
					{
						if(buff[i+j] != token[j]) 
						{
							break;
						}
					}
					//this should only be reached if a complete 'zoinks!' is found.
					malicious = 1;
				}
			}				
		}
	}
	
	if(malicious == 1) 
	{
		printf("Code is malicious! \n");	
	}
}
