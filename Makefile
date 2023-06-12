libusb-wrap.so: libusb-wrap.c
	gcc -fPIC -shared $< -o $@ -ldl

test: libusb-wrap.so
	LD_PRELOAD=$(abspath .)/libusb-wrap.so LIBUSB_WRAP_DEBUG=1 LIBUSB_WRAP_INCLUDE_VENDOR_ID=0x0c45 LIBUSB_WRAP_INCLUDE_PRODUCT_ID=0x670c lsusb
