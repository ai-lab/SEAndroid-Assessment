obj-m += sysMon.o

CROSS_COMPILE= /home/lucaverdio/seandroid-4.4.4/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-
KERNEL_DIR ?= /home/lucaverdio/seandroid-4.4.4/kernel/goldfish/

all:
	make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules
	rm -rf *.c~
	rm -rf *.mod*
	rm -rf *.o
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean


