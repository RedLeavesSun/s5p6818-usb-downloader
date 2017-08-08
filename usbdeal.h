#ifndef _USBDEAL_H_
#define _USBDEAL_H_

int usb_write(const uint8_t* buff, int size);
int usb_open(void);
void usb_close(void);

#endif
