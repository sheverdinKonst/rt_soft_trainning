#include <linux/module.h> /* Needed by all modules */ 
#include <linux/printk.h> /* Needed for pr_info() */ 
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/device.h> // Добавляем этот заголовочный файл
#include <linux/version.h>
#include <linux/kthread.h>

#include "../shared/ioct_driver.h"

char global_buff[1000];
static struct class *foo_class;
static struct device *foo_device;

dev_t dev = 0;
struct cdev *my_cdev;
unsigned int major;
static char flag = 0;

int val = 0;

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

    switch (cmd) 
    {
        case IOC_GET: 
         pr_info("--------------- >IOC_GET");
         long res_copy = copy_to_user(arg, &val, sizeof(val));
         pr_info("res_copy = %ld\n", res_copy);
         pr_info("flag = %d\n", flag);
         pr_info("arg = %ld", arg);
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

struct task_struct *ts;

int thread(void *data) 
{
    while(1) 
    {
        printk("Val = %d\n", val);
        msleep(100);
        if (kthread_should_stop())
        break;
    }
    return 0;
}

int init_module(void) 
{
    pr_info("***** 1 ************ init_module foo \n");

    memset(global_buff, 0, 1000);
    //init_waitqueue_head(&wq);
    if((alloc_chrdev_region(&dev, 0, 1, "foo")) < 0 )
    {
        //Allocating Major number
        return -1; // Cannot allocate major number for device
    }

    major = MAJOR(dev);
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
    my_cdev = cdev_alloc();
    my_cdev->ops = &fops;
    my_cdev->owner = THIS_MODULE;

    int res = cdev_add(my_cdev, dev, 1);
    if (res < 0)
    {
        unregister_chrdev_region(dev, 1);
        return res;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    foo_class = class_create(THIS_MODULE, "foo_class");
#else
    foo_class = class_create("foo_class");
#endif

    if(IS_ERR(foo_class))
    {
        goto r_class; // Cannot create the struct class for device
    }

    if(IS_ERR(device_create(foo_class,NULL,dev,NULL,"foo")))
    { // Creating device
        goto r_device; // Cannot create the Device
    }
    return 0; // Kernel Module Inserted Successfully
    r_device:
        class_destroy(foo_class);
    r_class:
        unregister_chrdev_region(dev,1);

    ts = kthread_run(thread, NULL, "foo kthread");
    printk("ts = %d\n", ts);
    return -1;
}

void cleanup_module(void) {
    device_destroy(foo_class, dev);
    class_destroy(foo_class);
    cdev_del(my_cdev);
    unregister_chrdev_region(dev, 1);
    kthread_stop(ts);
    pr_info("Kernel Module Removed Successfully...\n");
} 
 
MODULE_LICENSE("GPL");
