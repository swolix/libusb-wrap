libusb-wrap.so: libusb-wrap.c
	gcc -fPIC -shared $< -o $@ -ldl

test: libusb-wrap.so
	LD_PRELOAD=$(abspath .)/libusb-wrap.so lsusb
