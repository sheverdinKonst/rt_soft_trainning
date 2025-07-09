
#include <linux/pci.h>
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/printk.h> /* Needed for pr_info() */ 
#include <linux/kernel.h> /* Needed by all modules */ 

#include <linux/cdev.h>
#include <linux/device.h> // Добавляем этот заголовочный файл
#include <linux/version.h>

#include "../shared/ioct_driver.h"

char global_buff[1000];
static struct class *foo_class;
static struct device *foo_device;

dev_t dev_pci;
dev_t dev_sys;
struct cdev *my_cdev;
unsigned int major_pci;
unsigned int major_sys;

int  foo_probe(struct pci_dev *dev, const struct pci_device_id *id);
void foo_remove(struct pci_dev *dev);

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


//static struct pci_device_id rtl8168_pci_tbl[] = 
//{
//     {PCI_DEVICE{} }
    
    //{0x10ec, 0x8139, PCI_ANY_ID, PCI_ANY_ID, 0, 0, RTL8139 },
    //{0x10ec, 0x8138, PCI_ANY_ID, PCI_ANY_ID, 0, 0, RTL8139 },
    //{0,}
//};

static const struct pci_device_id cp_pci_tbl[] = {
    { PCI_DEVICE(0x10ec, 0x8168), },
    { },
};


MODULE_DEVICE_TABLE (pci, cp_pci_tbl);

static struct pci_driver rtl8168_pci_driver = {
    .name = "rtk_8168",
    .id_table = cp_pci_tbl,
    .probe = foo_probe,
    .remove = foo_remove,
};

const struct file_operations fops = 
{
   .read = device_read,
   //.write = device_write,
   .open = device_open,
   .release = device_release,
   //.unlocked_ioctl = device_ioctl
};

int foo_probe(struct pci_dev *pdev, const struct pci_device_id *id) // Реализация probe
{
    
    pr_info("***** 1 ************ init_module foo \n");

    memset(global_buff, 0, 1000);
    //init_waitqueue_head(&wq);
    if((alloc_chrdev_region(&dev_sys, 0, 1, "foo")) < 0 )
    {
        //Allocating Major number
        return -1; // Cannot allocate major number for device
    }

    major_sys = MAJOR(dev_sys);
    pr_info("Major = %d Minor = %d \n",MAJOR(dev_sys), MINOR(dev_sys));
    my_cdev = cdev_alloc();
    my_cdev->ops = &fops;
    my_cdev->owner = THIS_MODULE;

    int res = cdev_add(my_cdev, dev_sys, 1);
    if (res < 0)
    {
        unregister_chrdev_region(dev_sys, 1);
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

    if(IS_ERR(device_create(foo_class,NULL,dev_sys,NULL,"foo")))
    { // Creating device
        goto r_device; // Cannot create the Device
    }
    return 0; // Kernel Module Inserted Successfully
    r_device:
        class_destroy(foo_class);
    r_class:
        unregister_chrdev_region(dev_sys,1);
    
    
    int port_addr = pci_resource_start(pdev, 2);
    major_pci = register_chrdev(0, "rtk8168", &fops);
    printk(KERN_INFO KBUILD_MODNAME"Load driver PCI: rtk8168 %d\n", major_pci);

    // Запрашиваем ресурсы PCI
    int ret = pci_request_regions(pdev, "rtk_8168");
    if (ret) {
        pr_err("Failed to request PCI regions\n");
        pci_disable_device(pdev);
        return ret;
    }
     
    void __iomem *ioaddr = NULL;  

    // Получаем базовый адрес памяти (обычно BAR2 для сетевых карт)
    ioaddr = pci_iomap(pdev, 2, 0);
    if (!ioaddr) 
    {
        pr_err("Cannot map device registers\n");
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return -ENOMEM;
    }

    char mac_addr[6];
    int i = 0;
    // Читаем MAC-адрес (обычно находится по смещению 0x00)
    for (i = 0; i < 6; i++) {
        mac_addr[i] = ioread8(ioaddr + i);
    }

    // Печатаем MAC-адрес
    pr_info("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);

    return 0;
}

void foo_remove(struct pci_dev *pdev)
{
    device_destroy(foo_class, dev_sys);
    class_destroy(foo_class);
    cdev_del(my_cdev);
    unregister_chrdev_region(dev_sys, 1);
    pr_info("Kernel Module Removed Successfully...\n");
    
    printk(KERN_INFO KBUILD_MODNAME "UN_Load driver PCI\n");
    unregister_chrdev(major_pci,"rtk8168");
    printk(KERN_INFO KBUILD_MODNAME "UN_Load driver PCI\n");
}

int init_module(void) 
{
    return pci_register_driver(&rtl8168_pci_driver);
}

void cleanup_module(void)
{
    pci_unregister_driver(&rtl8168_pci_driver);
}

MODULE_LICENSE("GPL");