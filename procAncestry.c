/*evbruk & nparker
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <asm/current.h>

#include <asm/errno.h>


unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);

struct ancestry 
{
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, struct ancestry *response) {
	//do something with the target pid
	//populate the ancestry struct
	struct task_struct* currentTask;
	//currentTask = current;
	int pid = 0;
	
	struct ancestry localResponse;
	if( copy_from_user(&pid, target_pid, sizeof(int)) )
	{
		return EFAULT;	
	}	
	
	if( copy_from_user( &localResponse, response, sizeof(struct ancestry)) )
	{
		return EFAULT;	
	}

	struct pid * pid_struct;
	pid_struct = find_get_pid(pid); //get the pid struct for the request pid

	currentTask = pid_task(pid_struct, PIDTYPE_PID);
	struct task_struct* parentPointer = currentTask->real_parent;
	
	//this loop traverses to the top and breaks when a pid of 1 is found (top of process tree)
	while(parentPointer != parentPointer->real_parent) 
	{
		printk(KERN_INFO "PID: %d \n", parentPointer->pid);
		if(parentPointer->pid == 1)
		{
			break;		
		}
		if(parentPointer->pid > 0) 
		{
			parentPointer = parentPointer->real_parent;				
		}
	}
	
	int searching = 1;
	struct list_head* children = &parentPointer->children; //the head of the list
	
	struct list_head *position = LIST_HEAD_INIT(children);
	struct task_struct* child = 0;
	
	struct task_struct *child_type;

	//doesn't do anything
	int count = 0;	
	int max_children = 100;
	
	list_for_each_entry(child_type, children, sibling)
	{
		printk(KERN_INFO "Child traversed. pid: %d\n", child_type->pid);	
		if(count < max_children)
		{
			localResponse.children[count] = child_type->pid;
		}else{
			break;
		}
		count++;
	}
	
	//making a copy of the current Task
	struct task_struct * working_task = currentTask;
	int sibling_count = 0;
	int max_siblings = 100;
	list_for_each_entry(working_task, &(currentTask->sibling), sibling)
	{
		printk(KERN_INFO "sibling traversed, PID: %d\n", working_task->pid);
	}
	
	/*
	list_for_each(position, children)
	{
		printk(KERN_INFO "Before grabbing entry...\n"); 
		//child = list_entry(position, struct task_struct, children);
		//printk(KERN_INFO "Child traversed! PID: %d\n", child->pid); 
		
	}*
	
	
	
	/*	
	struct task_struct *task;
	struct task_struct *targetTask;
	printk(KERN_INFO "PID of parentPointer: %d \n", parentPointer->pid);
	
	int childIndex = 0;
	struct list_head *currentChildSearch; //pointer to current BFS child being searched.
	
	while(searching)
	{
		//this searches through each child of the top process
		list_for_each(list, &(parentPointer->children)) 
		{
			task=list_entry(list, struct task_struct, sibling);
			if(task->pid == pid)
			{
				printk(KERN_INFO "Specified task found! \n");
				targetTask = task;
				searching = 0;
				break;
			}
		}
		//Update the parentPointer to be the first child of the child, (BFS)

		//parentPointer = list_next_entry(currentChildSearch, parentPointer->children);
	
	}*/

	
	return 0;
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
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];   //if doesn't work, try changing to sys_open

  /* Replace the existing system calls */
  disable_page_protection();

  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
  
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
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
