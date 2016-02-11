#############################################################################
#
#	Makefile for building the Accelerator FPGA sample module
#
#############################################################################

MODULE_NAME := caffe_fpga_int

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m := $(MODULE_NAME).o

else

PWD := $(shell pwd)

default: kernel-module
.PHONY: kernel-module

kernel-module:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers $(MODULE_NAME)*

endif
