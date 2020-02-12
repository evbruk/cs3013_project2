//evbruch & nparker
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char * args[]) 
{
	//code to test the kernel output when reading a "malicious" file.
	FILE* filePointer = fopen("test_document", "r");
	char * buffer[256];
	fgets(buffer, 256, filePointer);
	fclose(filePointer);

	FILE* negFilePointer = fopen("test_document_nozoinks", "r");
	fgets(buffer, 256, negFilePointer);
	fclose(negFilePointer);
	
//	char * buff = "12345zoinks!sksjdlaksjd";
//	char * token = "zoinks!";
		
	
}
