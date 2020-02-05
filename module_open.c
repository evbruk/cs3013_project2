// module_open.c 
// ebruk & nparker 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

asmlinkage long new_sys_open(const char __user *filename,
				int flags, umode_t mode) 
{
	printk(KERN_INFO "***** Intercepted the open function! *****");
	return 0;
}


