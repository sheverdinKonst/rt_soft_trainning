obj-m += hello.o

KDIR := /work/2025_07/linux-6.1.80
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean