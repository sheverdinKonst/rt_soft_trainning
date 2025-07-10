obj-m += pci_dr_rtk8168.o

#KDIR := /work/2025_07/linux-6.1.80
#
#all:
#	make -C $(KDIR) M=$(PWD) modules
#clean:
#	make -C $(KDIR) M=$(PWD) clean


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
