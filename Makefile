obj-m += useless.o

helper:
	gcc useless_helper.c -o useless_helper -g

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
