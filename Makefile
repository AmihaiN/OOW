obj-m += inode_restorer.o

.PHONY: all clean

all: make_helper make_module

make_module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)

make_helper: restorer_helper.c
	gcc restorer_helper.c -o restorer_helper -g

clean: clean_helper clean_module

clean_module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

clean_helper:
	rm restorer_helper
