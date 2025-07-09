

#include <linux/module.h> /* Needed by all modules */ 
#include <linux/printk.h> /* Needed for pr_info() */ 
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/wait.h>>

char global_buff[1000];
dev_t dev = 0;
static struct class *dev_class;
struct cdev *my_cdev;
unsigned int major;
static char flag = 0;
wait_queue_head_t wq;

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
   pr_info("flag = %d\n", flag); 
   //if ((flag) && (filp->f_flags & O_NONBLOCK))
    //    return -EAGAIN;
   // if (wait_event_interruptible(wq, (ir != iw)))
   //     return -ERESTARTSYS;
   //pr_info("Wait fo data\n");
   wait_event_interruptible(wq, flag == 'y');
   flag = 1;

   pr_info("Data Recieved\n");
   
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

   wake_up_interruptible(&wq);  
   printk(KERN_INFO "Woken Up\n");
   //flag = 0;
   unsigned long write_byte = copy_from_user(global_buff, buffer, length); 
   pr_info("write_byte res  = %ld\n", write_byte); 
   return write_byte;
}

const struct file_operations fops = 
{
   .read = device_read,
   .write = device_write,
   .open = device_open,
   .release = device_release
};


int init_module(void) 
{

   pr_info("***** 1 ************ init_module foo \n");
   
   memset(global_buff, 0, 1000);
   init_waitqueue_head(&wq);
   if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0)
   {
     //Allocating Major number
        return -1; // Cannot allocate major number for device
   }

major = MAJOR(dev);
//pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
my_cdev = cdev_alloc();
my_cdev->ops = &fops;
my_cdev->owner = THIS_MODULE;

int res = cdev_add(my_cdev, dev, 1);
if (res < 0)
{
    unregister_chrdev_region(dev, 1);
    return res;  
}

dev_class = class_create(THIS_MODULE, "etx_class"); //Allocating Major number

if(IS_ERR(dev_class)){
    goto r_class; // Cannot create the struct class for device
}

if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"foo")))
{ // Creating device
goto r_device; // Cannot create the Device
}
    return 0; // Kernel Module Inserted Successfully
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
return -1; 

}

void cleanup_module(void) 
{ 
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(my_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Kernel Module Removed Successfully...\n");
}
 
MODULE_LICENSE("GPL");
