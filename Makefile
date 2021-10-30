obj-m := button-led.o


TARGET_MODULE:=button_led


ifneq ($(KERNELRELEASE),)
	$(TARGET_MODULE)-objs := button-led.o
	obj-m := $(TARGET_MODULE).o
else
	BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)
all:
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
install:
	insmod ./$(TARGET_MODULE).ko
delete:
	rmmod ./$(TARGET_MODULE).ko
endif
