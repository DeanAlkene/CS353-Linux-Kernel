#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <asm/current.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#define MAX_BUF_SIZE 64
static char proc_buf[MAX_BUF_SIZE];

static struct page* _find_page(struct vm_area_struct *vma, unsigned long addr)
{
    struct mm_struct *mm = vma->vm_mm;
    pgd_t *pgd = pgd_offset(mm, addr);
    p4d_t *p4d = NULL;
    pud_t *pud = NULL;
    pmd_t *pmd = NULL;
    pte_t *pte = NULL;
    struct page *page = NULL;

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
    page = pte_page(*pte);
    if(!page)
        return NULL;
    pte_unmap(pte);
    return page;
}

static void mtest_list_vma(void) 
{
    struct vm_area_struct *cur = current->mm->mmap;
    while(cur)
    {
        char perm[5] = "----";
        if(cur->vm_flags & VM_READ)
        {
            perm[0] = 'r';
        }
        if(cur->vm_flags & VM_WRITE)
        {
            perm[1] = 'w';
        }
        if(cur->vm_flags & VM_EXEC)
        {
            perm[2] = 'x';
        }
        if(cur->vm_flags & VM_SHARED)
        {
            perm[3] = 's';
        }
        printk(KERN_INFO "0x%lx\t0x%lx\t%s\n", cur->vm_start, cur->vm_end, perm);
        cur = cur->vm_next;
    }
}

static void mtest_find_page(unsigned long addr)
{
    struct vm_area_struct *vma = find_vma(current->mm, addr);
    struct page *page = NULL;
    unsigned long phy_addr;
    if (!vma)
    {
        printk(KERN_ERR "Translation not found\n");
        return;
    }
    page = _find_page(vma, addr);
    if(!page)
    {
        printk(KERN_ERR "Translation not found\n");
        return;
    }
    else
    {
        phy_addr = page_to_phys(page) | (addr & ~PAGE_MASK);
        printk(KERN_INFO "VA:0x%lx\tPA:0x%lx\n", addr, phy_addr);
    }
}

static void mtest_write_val(unsigned long addr, unsigned long val)
{
    struct vm_area_struct *vma = find_vma(current->mm, addr);
    struct page *page = NULL;
    unsigned long *kernel_addr;
    if (!vma)
    {
        printk(KERN_ERR "VMA not found\n");
        return;
    }
    if(!(vma->vm_flags & VM_WRITE))
    {
        printk(KERN_ERR "Writing permission denied\n");
        return;
    }
    page = _find_page(vma, addr);
    if(!page)
    {
        printk(KERN_ERR "Translation not found\n");
        return;
    }
    else
    {
        kernel_addr = (unsigned long *)page_address(page);
        *kernel_addr = val;
        printk(KERN_INFO "Write %ld into VA:0x%lx\n", val, addr);
    }
}

static ssize_t mtest_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    unsigned long int addr, val;
    if (copy_from_user(proc_buf, buffer, count))
    {
        printk(KERN_ERR "Copy from user unfinished\n");
        return -EFAULT;
    }

    if (strncmp(proc_buf, "listvma", 7) == 0)
    {
        printk(KERN_INFO "listvma\n");
        mtest_list_vma();
    }
    else if (strncmp(proc_buf, "findpage", 8) == 0)
    {
        sscanf(proc_buf + 9, "%lx", &addr);
        printk(KERN_INFO "findpage 0x%lx\n", addr);
        mtest_find_page(addr);
    }
    else if(strncmp(proc_buf, "writeval", 8) == 0)
    {
        sscanf(proc_buf + 9, "%lx %ld", &addr, &val);
        printk(KERN_INFO "writeval 0x%lx %ld\n", addr, val);
        mtest_write_val(addr, val);
    }
    else
    {
        printk(KERN_ERR "Invalid input!\n");
    }

    return count;
}

static struct file_operations proc_mtest_operations = {
    .write = mtest_proc_write
};

static struct proc_dir_entry *mtest_proc_entry = NULL;

static int __init mtest_init(void)
{
    mtest_proc_entry = proc_create("mtest", 0666, NULL, &proc_mtest_operations);
    if(!mtest_proc_entry)
    {
        printk(KERN_ERR "Unable to create /proc/mtest\n");
        return -EINVAL;
    }
    printk(KERN_INFO "/proc/mtest successfully created\n");
    return 0;
}

static void __exit mtest_exit(void)
{
    proc_remove(mtest_proc_entry);
    printk(KERN_INFO "Exiting mtest...");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module3");
MODULE_AUTHOR("Hongzhou Liu");
module_init(mtest_init);
module_exit(mtest_exit);
