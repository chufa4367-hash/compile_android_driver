
obj-m := Arlo.o
Arlo-objs := entry.o memory.o process.o

KDIR ?= "/storage/emulated/0/初罚kpm/android-gs-bluejay-5.10-android13"

PWD := $(shell pwd)


all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm64 LLVM=1 LLVM_IAS=1 modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm64 LLVM=1 clean
	rm -f *.ko *.o *.mod *.mod.c .*.cmd modules.order Module.symvers

.PHONY: all clean
