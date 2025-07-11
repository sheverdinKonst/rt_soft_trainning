obj-m += mmap_uart.o
KDIR := /work/2025_02/linux-6.6.68
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
