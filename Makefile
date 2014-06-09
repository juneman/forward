obj-m := forward2.o
	KERNELBUILD :=/lib/modules/$(shell uname -r)/build

all:
	gcc -g -o forward forward.c 
	make -C $(KERNELBUILD) M=$(shell pwd) modules

clean:
	rm -f forward
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions

