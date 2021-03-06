// module_open.c 
// ebruk & nparker 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_open)(const char __user *filename,
				int flags, umode_t mode);
asmlinkage long (*ref_sys_close)(unsigned int fd);
asmlinkage long (*ref_sys_read)(unsigned int fd, char __user *buf, size_t count);

asmlinkage long new_sys_read(unsigned int fd, char __user *buf, size_t count) 
{
	int malicious = 0;
	int linesRead = 0;
	//we are going to read from the file regardless of user.	
	linesRead = ref_sys_read(fd, buf, count);
	char * token = "zoinks!";
	
	
	if(current_uid().val >= 1000)
	{
		//iterate through buff, look for "zoinks!"
		if( count >= 7 ) 
		{
			int i = 0;
			for(i = 0; i < count; i++) 
			{
				//check against token.
				char currentChar = buf[i];
				if (currentChar == 'z')
				{
					//don't want to go out of the bounds of the buffer
					if(i + 7 <= count)                                			//should it be i + 6?
					{
						malicious = 1;
						int j = 0; 
						for(j = 0; j < 7; j++)
						{
							if(buf[i+j] != token[j]) 
							{
								malicious = 0;								
								break;
							}
						}
						//this should only be reached if a complete 'zoinks!' is found.
						
					}
				}				
			}
		}

		if( malicious )
		{
			printk(KERN_INFO "User read from file descriptor %ud, but that read contained malicious code! \n", fd);		
		}
		
	}

	return linesRead;
}

asmlinkage long new_sys_close(unsigned int fd) 
{
	if(current_uid().val >= 1000) 
	{
		printk(KERN_INFO "User %d is closing file descriptor: %u \n", current_uid().val, fd);
	}
	
	return ref_sys_close(fd);
}

asmlinkage long new_sys_open(const char __user *filename,
				int flags, umode_t mode) 
{
	if(current_uid().val >= 1000) 
	{
		printk(KERN_INFO "User %d is opening the file: %s\n", current_uid().val, filename);
	}

	
	return ref_sys_open(filename, flags, mode);
}

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
	     (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}


static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_open = (void *)sys_call_table[__NR_open];   
	ref_sys_close = (void *)sys_call_table[__NR_close];  
	ref_sys_read = (void *)sys_call_table[__NR_read];   
  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_open] = (unsigned long *)new_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)new_sys_close;  
	sys_call_table[__NR_read] = (unsigned long *)new_sys_read;

  enable_page_protection();
  
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
	sys_call_table[__NR_read] = (unsigned long *)ref_sys_read;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);

