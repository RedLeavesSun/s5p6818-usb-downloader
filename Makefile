all:
	gcc main.c usbdeal.c -g -I /usr/include/libusb-1.0 -lusb-1.0 -L /usr/local/lib -o s5p6818_usb_downloader