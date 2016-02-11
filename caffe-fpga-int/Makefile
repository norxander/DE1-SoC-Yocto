#############################################################################
#
#	Makefile for building the Accelerator FPGA sample module
#
#############################################################################

obj-m := fpga_hps_int.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers fpga_hps_int

ifeq (.depend,$(wildcard .depend))
include .depend
endif
