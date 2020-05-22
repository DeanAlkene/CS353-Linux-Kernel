#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

static void mtest_list_vma(void) 
{

}

static void mtest_find_page(unsigned long addr)
{

}

static void mtest_write_val(unsigned long addr, unsigned long val)
{

}

static ssize_t mtest_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{

}

static struct file_operations proc_mtest_operations = {
    .write = mtest_proc_write
};

static struct proc_dir_entry *mtest_proc_entry;

static int __init mtest_init(void)
{

}

static void __exit mtest_exit(void)
{

}

module_init(mtest_init)
module_exit(mtest_exit)
