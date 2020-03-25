#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
static int int_param;
static char* str_param;
static char* arr_param_fake;
static int arr_num_fake = 0;
static int* arr_fake;
static int arr_param[5];
static int arr_argc = 0;
module_param(int_param, int, 0644);
module_param(str_param, charp, 0644);
module_param_array(arr_param, int, &arr_argc, 0644);
module_param(arr_param_fake, charp, 0644);

static int __init M2_init(void)
{
    int i = 0, tmp = 0;
    char* cp = arr_param_fake;
    printk(KERN_INFO "int_param = %d\n", int_param);
    printk(KERN_INFO "str_param = %s\n", str_param);
    for(i = 0; i < sizeof(arr_param) / sizeof(int); ++i)
    {
        printk(KERN_INFO "arr_param[%d] = %d\n", i, arr_param[i]);
    }
    printk(KERN_INFO "got %d input array elements\n", arr_argc);
    
    if(arr_param_fake)
    {
        arr_fake = (int*)kmalloc_array(strlen(arr_param_fake), sizeof(int), GFP_KERNEL);
        while(*cp)
        {
            while(*cp >= '0' && *cp <= '9')
            {
                tmp *= 10;
                tmp += *cp - '0';
                cp++;
            }
            arr_fake[arr_num_fake++] = tmp;
            tmp = 0;
            cp++;
        }
        for(i = 0; i < arr_num_fake; ++i)
        {
            printk(KERN_INFO "arr_fake[%d] = %d\n", i, arr_fake[i]);
        }
        printk(KERN_INFO "got %d input array elements using char* as input\n", arr_num_fake);
        kfree(arr_fake);    
    }
    else
    {
        printk(KERN_INFO "arr_fake = (null)\n");
    }
    return 0;
}

static void __exit M2_exit(void)
{
    printk(KERN_INFO "Exiting Module2...\n");
}
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module2");
MODULE_AUTHOR("Hongzhou Liu");
module_init(M2_init);
module_exit(M2_exit);