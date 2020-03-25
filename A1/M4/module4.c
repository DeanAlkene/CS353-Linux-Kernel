#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#define MAX_BUF_SIZE 2048
char *msg;
static char proc_buf[MAX_BUF_SIZE];
static unsigned long proc_buf_size = 0;

static ssize_t read_proc(struct file *filp, char *usr_buf, size_t count, loff_t *offp) 
{
    static int finished = 0;
    if(finished)
    {
        printk(KERN_INFO "read_proc: END\n");
        finished = 0;
        return 0;
    }
    finished = 1;
    if(copy_to_user(usr_buf, proc_buf, proc_buf_size))
    {
        printk(KERN_ERR "Copy to user unfinished\n");
        return -EFAULT;
    }
    printk(KERN_INFO "read_proc: read %lu bytes\n", proc_buf_size);
    return proc_buf_size;
}

static ssize_t write_proc(struct file *filp, const char *usr_buf, size_t count, loff_t *offp) 
{
    if(count > MAX_BUF_SIZE)
    {
        proc_buf_size = MAX_BUF_SIZE;
    }
    else
    {
        proc_buf_size = count;
    }
    
    if(copy_from_user(proc_buf, usr_buf, proc_buf_size))
    {
        printk(KERN_ERR "Copy from user unfinished\n");
        return -EFAULT;
    }
    printk(KERN_INFO "write_proc: write %lu bytes\n", proc_buf_size);
    return proc_buf_size;
}

struct file_operations proc_fops = { .read = read_proc, .write = write_proc };
struct proc_dir_entry *base = NULL;
struct proc_dir_entry *entry = NULL;

static int __init M4_init (void) 
{
    base = proc_mkdir("M4_proc_dir", NULL);
    if(!base)
    {
        printk(KERN_ERR "Unable to create /proc/M4_proc_dir/\n");
        return -EINVAL;
    }
    entry = proc_create("M4_proc", 0666, base, &proc_fops);
    if(!entry)
    {
        printk(KERN_ERR "Unable to create /proc/M4_proc_dir/M4_proc\n");
        proc_remove(base);
        return -EINVAL;
    }
    printk(KERN_INFO "/proc/M4_proc_dir/M4_proc successfully created\n");
    msg = "Hello /proc!\n";
    strcpy(proc_buf, msg);
    proc_buf_size = strlen(msg);
    return 0;
}

void __exit M4_exit(void) 
{
    proc_remove(entry);
    printk(KERN_INFO "Exiting Module4...\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module4");
MODULE_AUTHOR("Hongzhou Liu");
module_init(M4_init);
module_exit(M4_exit);
