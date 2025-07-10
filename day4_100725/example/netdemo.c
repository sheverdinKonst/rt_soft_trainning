#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>

struct net_device* demo_nic;

MODULE_LICENSE("GPL");

int demo_nic_open(struct net_device *dev) {
  pr_info("demo_nic open\n");
  return 0;
}

int demo_nic_release(struct net_device *dev) {
  pr_info("demo_nic release\n");
  netif_stop_queue(dev);
  return 0;
}

int demo_nic_xmit(struct sk_buff *skb, struct net_device *dev) {
 pr_info("demo_nic xmit\n");

 dev->stats.tx_bytes += skb->len;
 dev->stats.tx_packets++;

 pr_info("%pTN<struct sk_buff>", skb);

 dev_kfree_skb(skb);
 return 0;
}

int demo_nic_init(struct net_device *dev) {
  pr_info("demo_nic initialized\n");

  return 0;
};


const struct net_device_ops demo_nic_netdev_ops = {
     .ndo_init = demo_nic_init,
     .ndo_open = demo_nic_open,
     .ndo_stop = demo_nic_release,
     .ndo_start_xmit = demo_nic_xmit,
};


static void demo_nic_setup(struct net_device *dev){
  dev->netdev_ops = &demo_nic_netdev_ops;
}

int init_module(void) {
  int result;

  // demo_nic = alloc_netdev(0, "foonet%d", NET_NAME_UNKNOWN, demo_nic_setup);

  demo_nic = alloc_etherdev(0);
  demo_nic->netdev_ops = &demo_nic_netdev_ops; 
  if((result = register_netdev(demo_nic))) {
    pr_info("demo_nic: Error %d initalizing card ...", result);
    return result;
  }
  return 0;
}

void cleanup_module (void)
{
  pr_info("Cleaning Up the Module\n");
  unregister_netdev (demo_nic);
}


