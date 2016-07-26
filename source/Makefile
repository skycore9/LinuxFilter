INCDIR=$(PWD)/../include

obj-m := TTCN_LinuxFilter.o
ccflags-y := -I$(INCDIR)
TTCN_LinuxFilter-y := TTCN_DispatchLayer.o TTCN_PreProcess.o \
					  TTCN_SessionManage.o TTCN_UserLevel.o \
					  TTCN_AppDetect.o TTCN_BandwidthManage.o \
					  TTCN_CommandParse.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
