#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int __init M1_init(void)
{
    printk(KERN_INFO "Hello Linux Kernel!\n");
    return 0;
}

static void __exit M1_exit(void)
{
    printk(KERN_INFO "Goodbye Linux Kernel!\n");
}
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module2");
MODULE_AUTHOR("Hongzhou Liu");
module_init(M1_init);
module_exit(M1_exit);
