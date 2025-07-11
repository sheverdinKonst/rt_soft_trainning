#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/serial_reg.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/irqreturn.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/cdev.h>


unsigned long mmio_base = 0;

int hw_mmap( struct file *file, struct vm_area_struct *vma ){
  unsigned long	region_origin = vma->vm_pgoff * PAGE_SIZE;
  unsigned long	region_length = vma->vm_end - vma->vm_start;
  unsigned long	physical_addr = mmio_base + region_origin;
  unsigned long	user_virtaddr = vma->vm_start;
  unsigned long	phys_page_off = physical_addr & ~PAGE_MASK; // determine page-offset 

  if ( region_length > PAGE_SIZE ) return -EINVAL; // sanity check: mapped region is confined to just one page 
       vm_flags_set(vma, VM_IO |VM_DONTDUMP);

	// ask the kernel to set up the required page-tables
//	if ( io_remap_page_range( vma, user_virtaddr, physical_addr>>PAGE_SHIFT,
	if ( io_remap_pfn_range( vma, user_virtaddr, physical_addr>>PAGE_SHIFT,region_length, 
	        vma->vm_page_prot ) ) return -EAGAIN;	
		vma->vm_start += phys_page_off; // add page offset to virtual page start 
 	return	0;  // SUCCESS
}
/*********************************************************/
static int hw_open(struct inode *inode, struct file *file)
{
    return 0;
}
/*********************************************************/
static int hw_close(struct inode *inodep, struct file *filp)
{
    return 0;
}
/*********************************************************/

/*********************************************************/


int Major;
static struct class *dev_class;
dev_t dev; 


static const struct file_operations hw_fops = {
    .owner = THIS_MODULE,
    .open = hw_open,
    .release = hw_close,
    .mmap = hw_mmap,
};


/*********************************************************/
static int hw_probe(struct platform_device *pdev)
{
    struct resource *iores;
    iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);

    if (!iores)
    {
        pr_err("%s: platform_get_resource returned NULL\n", __func__);
        return -EINVAL;
    }

    //dev->regs = devm_ioremap_resource(&pdev->dev, res);
    mmio_base = iores->start;

	int res;
	  
	if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
      	//Allocating Major number
		return -1; // Cannot allocate major number for device
	}
	Major = MAJOR(dev);
	struct cdev* my_cdev;
	
	my_cdev = cdev_alloc();
	my_cdev->ops = &hw_fops;
	my_cdev->owner = THIS_MODULE;
	
	res = cdev_add(my_cdev, dev, 1);
	if (res<0){
		unregister_chrdev_region(dev, 1);
		return res;
	}


    	if (Major < 0) {
       		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
       		return Major;
    	}
    	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
    
    	printk(KERN_INFO "Hello world.\n");
    	
        dev_class = class_create("etx_class"); 
	//Allocating Major number
	if(IS_ERR(dev_class)){
		goto r_class; // Cannot create the struct class for device
	}
	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){ // Creating device
		goto r_device; // Cannot create the Device
	}
	return 0; // Kernel Module Inserted Successfully
	r_device:
		class_destroy(dev_class);
	r_class:
		unregister_chrdev_region(dev,1);
		return -1;

    return 0;
}

/*********************************************************/
static int hw_remove(struct platform_device *pdev)
{
   device_destroy(dev_class,dev);
   class_destroy(dev_class);
   unregister_chrdev_region(dev, 1);

   return 0;
}

//Device id struct
static struct of_device_id hw_match_table[] =
    {
        {
            .compatible = "serial",
        },
};

//File operations struct

//Platform driver structure
static struct platform_driver hw_plat_driver = {
    .driver = {
        .name = "serial",
        .owner = THIS_MODULE,
        .of_match_table = hw_match_table},
    .probe = hw_probe,
    .remove = hw_remove
};

MODULE_LICENSE("GPL");
module_platform_driver(hw_plat_driver);
