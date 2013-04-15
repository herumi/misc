/*
	make -C /lib/modules/`uname -r`/build M=`pwd`
	see kernel/kmod.c
*/
#include <linux/module.h>
#include <linux/kmod.h>

char usercmd[256] = "/bin/ls";

static int __init call_user_init(void)
{
	int ret;
	char *argv[] = { usercmd, NULL };
	char *envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
	ret = call_usermodehelper(usercmd, argv, envp, UMH_WAIT_PROC);
	if (ret) {
		printk(KERN_INFO "call_user_init fail=%d\n", ret);
	}
	return ret;
}

static void __exit call_user_exit(void)
{
	printk(KERN_INFO "call_user_exit\n");
}

module_init(call_user_init);
module_exit(call_user_exit);

MODULE_AUTHOR("MITSUNARI Shigeo");
MODULE_LICENSE("GPL");

