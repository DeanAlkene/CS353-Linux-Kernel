#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/pid.h>
#include <linux/timekeeping.h>
#include <linux/ktime.h>
#include <asm/current.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#define PAGE_NUM 1000000
#define MAX_READ_BUF_SIZE 5000000
#define MAX_WRITE_BUF_SIZE 64
static char proc_buf[MAX_WRITE_BUF_SIZE];
static unsigned long malloc_num = 0;
static int is_show_malloc = 0;

struct heat
{
    unsigned long addr;
    int access_time;
};

static struct heat heat_info[PAGE_NUM + 1];
static int heat_info_len = 0;

static inline pte_t* _find_pte(struct mm_struct *mm, unsigned long addr)
{
    pgd_t *pgd = NULL;
    p4d_t *p4d = NULL;
    pud_t *pud = NULL;
    pmd_t *pmd = NULL;
    pte_t *pte = NULL;

    pgd = pgd_offset(mm, addr);
    if(pgd_none(*pgd) || pgd_bad(*pgd))
        return NULL;

    p4d = p4d_offset(pgd, addr);
    if(p4d_none(*p4d) || p4d_bad(*p4d))
        return NULL;

    pud = pud_offset(p4d, addr);
    if(pud_none(*pud) || pud_bad(*pud))
        return NULL;

    pmd = pmd_offset(pud, addr);
    if(pmd_none(*pmd) || pmd_bad(*pmd))
        return NULL;

    pte = pte_offset_map(pmd, addr);
    if(pte_none(*pte) || !pte_present(*pte))
        return NULL;
    return pte;
}

static int filter_page(pid_t pid)
{
    struct pid *p;
    struct task_struct *tsk;
    struct vm_area_struct *cur;
    unsigned long filtered_num, addr;

    if (pid < 0)
    {
        printk(KERN_ERR "Invalid PID\n");
        return -EFAULT;
    }

    p = find_get_pid(pid);

    if (!p)
    {
        printk(KERN_ERR "Invalid pid\n");
        return -EFAULT;
    }

    tsk = get_pid_task(p, PIDTYPE_PID);

    if (!tsk)
    {
        printk(KERN_ERR "Invalid task_struct\n");
        return -EFAULT;
    }

    if (!tsk->mm)
    {
        printk(KERN_ERR "task_struct->mm is NULL\n");
        return -EFAULT;
    }

    cur = tsk->mm->mmap;
    filtered_num = 0;
    heat_info_len = 0;

    while (cur)
    {
        if (cur->vm_flags & (VM_WRITE & ~VM_SHARED & ~VM_STACK))
        {
            printk(KERN_INFO "FILTERED: 0x%lx\t0x%lx\n", cur->vm_start, cur->vm_end);
            for (addr = cur->vm_start; addr < cur->vm_end; addr += PAGE_SIZE)
            {
                heat_info[heat_info_len + 1].addr = addr;
                heat_info[heat_info_len + 1].access_time = 0;
                heat_info_len++;
            }
            filtered_num += vma_pages(cur);
        }
        cur = cur->vm_next;
    }
    heat_info[0].addr = 0;
    heat_info[0].access_time = filtered_num;
    if (is_show_malloc)
        printk(KERN_INFO "total: %ld, filtered: %ld, malloc: %ld\n", tsk->mm->total_vm, filtered_num, malloc_num);
    else
        printk(KERN_INFO "total: %ld, filtered: %ld\n", tsk->mm->total_vm, filtered_num);
    
    return 0;
}

static int collect_heat(pid_t pid) 
{
    struct pid *p;
    struct task_struct *tsk;
    pte_t *pte;
    int i;
    ktime_t calltime, delta, rettime;
    unsigned long long duration;

    if (pid < 0)
    {
        printk(KERN_ERR "Invalid PID\n");
        return -EFAULT;
    }

    p = find_get_pid(pid);

    if (!p)
    {
        printk(KERN_ERR "Invalid pid\n");
        return -EFAULT;
    }

    tsk = get_pid_task(p, PIDTYPE_PID);

    if (!tsk)
    {
        printk(KERN_ERR "Invalid task_struct\n");
        return -EFAULT;
    }

    if (!tsk->mm)
    {
        printk(KERN_ERR "task_struct->mm is NULL\n");
        return -EFAULT;
    }

    calltime = ktime_get();
    for (i = 0; i < heat_info_len; ++i)
    {
        pte = _find_pte(tsk->mm, heat_info[i + 1].addr);
        if (!pte)
        {
            continue;
        }
        if (pte_young(*pte))
        {
            heat_info[i + 1].access_time += 1;
            *pte = pte_mkold(*pte);
        }
    }
    rettime = ktime_get();
    delta = ktime_sub(rettime, calltime);
    duration = (unsigned long long)ktime_to_us(delta);
    printk(KERN_INFO "Heat collection elapsed after %lld us\n", duration);
    return 0;
}

static void print_info(void)
{
    int i;
    for (i = 0; i < heat_info_len; ++i)
    {
        printk(KERN_INFO "PAGE: 0x%lx\theat: %d\n", heat_info[i + 1].addr, heat_info[i + 1].access_time);
    }
}

static ssize_t getHeat_proc_read(struct file *filp, char *usr_buf, size_t count, loff_t *offp) 
{
    if (copy_to_user(usr_buf, heat_info, sizeof(heat_info)))
    {
        printk(KERN_ERR "Copy to user unfinished\n");
        return -EFAULT;
    }
    return sizeof(heat_info);
}

static ssize_t getHeat_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    int pid;
    int ret;
    if (copy_from_user(proc_buf, buffer, count))
    {
        printk(KERN_ERR "Copy from user unfinished\n");
        return -EFAULT;
    }
    proc_buf[count] = '\0';

    if (strncmp(proc_buf, "filter", 6) == 0)
    {
        sscanf(proc_buf + 7, "%d", &pid);
        printk(KERN_INFO "filter PID: %d\n", pid);
        ret = filter_page(pid);
    }
    else if (strncmp(proc_buf, "collect", 7) == 0)
    {
        sscanf(proc_buf + 8, "%d", &pid);
        printk(KERN_INFO "collect PID: %d\n", pid);
        ret = collect_heat(pid);
    }
    else if (strncmp(proc_buf, "malloc", 6) == 0)
    {
        sscanf(proc_buf + 7, "%d %ld", &is_show_malloc, &malloc_num);
    }
    else if (strncmp(proc_buf, "print", 5) == 0)
    {
        print_info();
    }
    else
    {
        printk(KERN_ERR "Invalid input!\n");
    }
    
    return count;
}

static struct file_operations proc_getHeat_operations = {
    .read = getHeat_proc_read,
    .write = getHeat_proc_write
};

static struct proc_dir_entry *getHeat_proc_entry = NULL;

static int __init getHeat_init(void)
{
    getHeat_proc_entry = proc_create("heat", 0666, NULL, &proc_getHeat_operations);
    if(!getHeat_proc_entry)
    {
        printk(KERN_ERR "Unable to create /proc/heat\n");
        return -EINVAL;
    }
    printk(KERN_INFO "/proc/heat successfully created\n");
    return 0;
}

static void __exit getHeat_exit(void)
{
    proc_remove(getHeat_proc_entry);
    printk(KERN_INFO "Exiting getHeat...");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Final");
MODULE_AUTHOR("Hongzhou Liu");
module_init(getHeat_init);
module_exit(getHeat_exit);
