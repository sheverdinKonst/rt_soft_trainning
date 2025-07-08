
#include <linux/pci.h>
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/printk.h> /* Needed for pr_info() */ 
#include <linux/kernel.h> /* Needed by all modules */ 


MODULE_LICENSE("GPL");
int foo_probe(struct pci_dev *dev, const struct pci_device_id *id);
void foo_remove(struct pci_dev *dev);


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

int foo_probe(struct pci_dev *dev, const struct pci_device_id *id) // Реализация probe
{
    //port_addr = pci_resource_start(dev,0);
    //major = register_chrdev(0,"MyPCI",&fops);
    //printk(KERN_INFO "Load driver PCI %d\n", major);
    printk(KERN_DEBUG KBUILD_MODNAME "Load driver PCI\n");
    return 0;
}

void foo_remove(struct pci_dev *dev)
{
    printk(KERN_INFO KBUILD_MODNAME "UN_Load driver PCI\n");
    //unregister_chrdev(major,"MyPCI");
}

int init_module(void) 
{
    return pci_register_driver(&rtl8168_pci_driver);
}

void cleanup_module(void)
{
    pci_unregister_driver(&rtl8168_pci_driver);
}