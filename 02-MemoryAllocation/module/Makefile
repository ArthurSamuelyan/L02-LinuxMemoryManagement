MODULE_DIR		:= /home/arthur/Projects/KernelDev/L02-LinuxMemoryManagement/02-MemoryAllocation/module
obj-m 			+= alloc-time-cdev.o


all:
	  make -C /lib/modules/$(shell uname -r)/build M=$(MODULE_DIR) modules

clean:
	  make -C /lib/modules/$(shell uname -r)/build M=$(MODULE_DIR) clean

