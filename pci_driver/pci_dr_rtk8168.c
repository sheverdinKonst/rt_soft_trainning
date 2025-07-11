#include <linux/pci.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ctype.h>

#include "../shared/ioct_driver.h"

char global_buff[1000];
static struct class *foo_class;
static struct device *foo_device;

dev_t dev_pci;
dev_t dev_sys;
struct cdev *my_cdev;
unsigned int major_pci;
unsigned int major_sys;

unsigned int irq = 0;

int foo_probe(struct pci_dev *dev, const struct pci_device_id *id);
void foo_remove(struct pci_dev *dev);

static const struct pci_device_id cp_pci_tbl[] = {
    { PCI_DEVICE(0x10ec, 0x8168), },
    { },
};

MODULE_DEVICE_TABLE(pci, cp_pci_tbl);

struct net_device* net_dev;

irqreturn_t my_isr(int irq, void *dev_id)
{
    pr_info("IRQ for Net device\n");
    return IRQ_HANDLED;
}

int demo_nic_init(struct net_device *dev)
{
    pr_info("demo_nic initialized\n");
    return 0;
}

int demo_nic_open(struct net_device *dev)
{
    pr_info("demo_nic open\n");
    netif_start_queue(dev);

    if (!dev) 
    {   
        pr_info("return -ENODEV\n");
        return -ENODEV;
    }
        
    irq = dev->irq;

    pr_info("irq = %d\n", irq);

    if (request_irq(irq, my_isr, IRQF_SHARED, "eth_0", dev))
    {
        pr_info("return -EBUSY\n");
        //return -EBUSY;
    }
    return 0;
}

int demo_nic_release(struct net_device *dev)
{
    pr_info("demo_nic release\n");
    free_irq(irq, (void*) dev);
    netif_stop_queue(dev);
    return 0;
}

int demo_nic_xmit(struct sk_buff *skb, struct net_device *dev) 
{
    struct ethhdr *eth;
    struct iphdr *iph;
    struct tcphdr *tcph;
    struct udphdr *udph;
    unsigned char *payload;
    int i, payload_len;
    
    pr_info("\n=== Packet received for transmission ===\n");
    
    // Update statistics
    dev->stats.tx_bytes += skb->len;
    dev->stats.tx_packets++;
    
    // Print basic packet info
    pr_info("Packet length: %u\n", skb->len);
    
    // Ethernet header
    eth = (struct ethhdr *)skb_mac_header(skb);
    pr_info("Ethernet: src: %pM dst: %pM proto: 0x%04x\n", 
           eth->h_source, eth->h_dest, ntohs(eth->h_proto));
    
    // IP header (if available)
    //if (ntohs(eth->h_proto) == ETH_P_IP) {
    //    iph = (struct iphdr *)skb_network_header(skb);
    //    pr_info("IP: src: %pI4 dst: %pI4 proto: %d TTL: %d\n",
    //           &iph->saddr, &iph->daddr, iph->protocol, iph->ttl);
    //
    //    // TCP header
    //    if (iph->protocol == IPPROTO_TCP) {
    //        tcph = (struct tcphdr *)skb_transport_header(skb);
    //        pr_info("TCP: sport: %d dport: %d seq: %u ack: %u\n",
    //               ntohs(tcph->source), ntohs(tcph->dest),
    //               ntohl(tcph->seq), ntohl(tcph->ack_seq));
    //
    //        // Calculate payload
    //        payload = (unsigned char *)(tcph) + (tcph->doff * 4);
    //        payload_len = ntohs(iph->tot_len) - (iph->ihl * 4) - (tcph->doff * 4);
    //    }
    //    // UDP header
    //    else if (iph->protocol == IPPROTO_UDP) {
    //        udph = (struct udphdr *)skb_transport_header(skb);
    //        pr_info("UDP: sport: %d dport: %d len: %d\n",
    //               ntohs(udph->source), ntohs(udph->dest), ntohs(udph->len));
    //
    //        // Calculate payload
    //        payload = (unsigned char *)(udph) + sizeof(struct udphdr);
    //        payload_len = ntohs(udph->len) - sizeof(struct udphdr);
    //    }
    //    // Other IP protocols
    //    else {
    //        payload = (unsigned char *)skb_transport_header(skb);
    //        payload_len = ntohs(iph->tot_len) - (iph->ihl * 4);
    //    }
    //
    //    // Print payload (first 64 bytes)
    //    if (payload_len > 0) {
    //        pr_info("Payload (%d bytes):\n", payload_len);
    //        for (i = 0; i < min(payload_len, 64); i++) {
    //            if (i % 16 == 0) pr_cont("\n%04x: ", i);
    //            pr_cont("%02x ", payload[i]);
    //        }
    //        pr_cont("\n");
    //
    //        // Print as ASCII (if printable)
    //        pr_info("ASCII:\n");
    //        for (i = 0; i < min(payload_len, 64); i++) {
    //            if (i % 16 == 0) pr_cont("\n%04x: ", i);
    //            pr_cont("%c ", isprint(payload[i]) ? payload[i] : '.');
    //        }
    //        pr_cont("\n");
    //    }
    //}
    //// Non-IP packet
    //else {
    //    payload = skb->data + sizeof(struct ethhdr);
    //    payload_len = skb->len - sizeof(struct ethhdr);
    //
    //    if (payload_len > 0) {
    //        pr_info("Non-IP payload (%d bytes):\n", payload_len);
    //        for (i = 0; i < min(payload_len, 64); i++) {
    //            if (i % 16 == 0) pr_cont("\n%04x: ", i);
    //            pr_cont("%02x ", payload[i]);
    //        }
    //        pr_cont("\n");
    //    }
    //}
    
    dev_kfree_skb(skb);
    return NETDEV_TX_OK;
}

const struct net_device_ops demo_nic_netdev_ops = 
{
    .ndo_init = demo_nic_init,
    .ndo_open = demo_nic_open,
    .ndo_stop = demo_nic_release,
    .ndo_start_xmit = demo_nic_xmit,
};

static void demo_nic_setup(struct net_device *dev)
{
    dev->netdev_ops = &demo_nic_netdev_ops;
}

void __iomem *ioaddr = NULL;

int foo_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    pr_info("***** NET ************ init_module foo \n");

    unsigned long pci_addr = pci_resource_start(pdev, 2);
    unsigned long pci_len = pci_resource_len(pdev, 2);
    
    if ((pci_addr == 0) || (pci_len == 0)) {   
        printk("!!! pci_resource_start - failed\n");
        return -1;
    }
    else {
        printk("%u...OK.\n", (int)pci_addr);
    }

    if (pci_resource_flags(pdev, 2) & IORESOURCE_MEM) {
        printk("pci_resource_flags OK\n");
    }
    else {
        printk("pci_resource_flags failed\n");
        return -1;
    }
    
    printk("Get virtual BAR 2...");
    ioaddr = ioremap(pci_addr, pci_len);
    if (!ioaddr) {
        printk("ioremap failed\n");
        return -1;
    }
    else {
        printk("%u...OK.\n", (int)ioaddr);
    }

    printk("Request region BAR 2 ... \n");

    //if 
    request_mem_region(pci_addr, pci_len, "rtk_8168");
    //{
    //    printk("request_mem_region failed\n");
    //    iounmap(ioaddr);
    //    return -1;
    //}
    printk("request_mem_region OK\n");

    unsigned char mac_addr[6];
    int i;
    for (i = 0; i < 6; i++) {
        mac_addr[i] = ioread8(ioaddr + i);
    }

    pr_info("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);

    int result;
    printk("NET DRIVER INIT\n");

    net_dev = alloc_etherdev(0);
    net_dev->irq = pdev->irq;
    pr_info("net_dev->irq = %d\n", net_dev->irq);
    if (!net_dev) {
        pr_info("Failed to allocate net device\n");
        release_mem_region(pci_addr, pci_len);
        iounmap(ioaddr);
        return -ENOMEM;
    }

    unsigned char custom_mac[ETH_ALEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    memcpy(net_dev->dev_addr, custom_mac, ETH_ALEN);
    net_dev->netdev_ops = &demo_nic_netdev_ops;
    net_dev->mtu = ETH_DATA_LEN;

    if ((result = register_netdev(net_dev))) {
        pr_info("net_dev: Error %d initializing card ...", result);
        free_netdev(net_dev);
        release_mem_region(pci_addr, pci_len);
        iounmap(ioaddr);
        return result;
    }
    
    pr_info("Network device registered with MAC: %pM\n", net_dev->dev_addr);
    return 0;
}

void foo_remove(struct pci_dev *pdev)
{
    if (net_dev) {
        unregister_netdev(net_dev);
        free_netdev(net_dev);
    }
    
    if (ioaddr) {
        iounmap(ioaddr);
        release_mem_region(pci_resource_start(pdev, 2), pci_resource_len(pdev, 2));
    }
    
    printk(KERN_INFO KBUILD_MODNAME " >> UN_Load driver PCI\n");
}

static struct pci_driver rtl8168_pci_driver = {
    .name = "rtk_8168",
    .id_table = cp_pci_tbl,
    .probe = foo_probe,
    .remove = foo_remove,
};

int init_module(void)
{
    return pci_register_driver(&rtl8168_pci_driver);
}

void cleanup_module(void)
{
    pci_unregister_driver(&rtl8168_pci_driver);
}

MODULE_LICENSE("GPL");