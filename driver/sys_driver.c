#include <linux/module.h> /* Needed by all modules */ 
#include <linux/printk.h> /* Needed for pr_info() */ 
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>

#include "../shared/ioct_driver.h"


char global_buff[1000];

static int device_open(struct inode *inode, struct file *file)
{
   pr_info("device_open foo \n"); 
   return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
  pr_info("device_release foo \n"); 
  return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
   pr_info("read_byte length = %ld\n", length); 

   if (length > 1000)
     length = 1000;
   unsigned long read_byte = copy_to_user(buffer, global_buff, length);
   pr_info("read_byte = %ld\n", read_byte); 
   return length;
}

// ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);

static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t * offset)
{
   pr_info("write_byte length = %ld\n", length); 
   if (length > 1000)
     length = 1000;   
   unsigned long write_byte = copy_from_user(global_buff, buffer, length); 
   pr_info("write_byte = %ld\n", write_byte); 
   return write_byte;
}

//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
long device_ioctl(struct file *filp,  unsigned int cmd, unsigned long arg) 
{
    long ret=0;
    pr_info(">>>>>>> device_ioctl");
    pr_info("cmd = %d", IOC_GET);
    pr_info("arg = %ld", arg);

    switch (cmd) 
    {
        case IOC_GET: 
         pr_info("--------------- >IOC_GET");
         ret = 0;
         break;
        //case IOC_SET: 
        // break;
        default: //return -EINVAL; // old style
         return -ENOTTY;
    }
    return ret;
}

const struct file_operations fops = 
{
   .read = device_read,
   .write = device_write,
   .open = device_open,
   .release = device_release,
   .unlocked_ioctl = device_ioctl
};

unsigned int Major;

int init_module(void) 
{

   pr_info("***** 1 ************ init_module foo \n");
   
   memset(global_buff, 0, 1000);
   Major = register_chrdev(0, "foo", &fops);
   if (Major < 0) {
       printk(KERN_ALERT "Registering char device failed with %d\n", Major);
       return Major;
   }

    /* A non 0 return means init_module failed; module can't be loaded. */
    return 0;
}

void cleanup_module(void) { 
   pr_info("cleanup_module foo  1.\n"); 
   unregister_chrdev(Major, "foo");
} 
 
MODULE_LICENSE("GPL");
