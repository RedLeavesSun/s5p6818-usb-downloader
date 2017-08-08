#include <libusb.h>
#include <stdint.h>

#define S5P6818_VID         0x04e8
#define S5P6818_PID         0x1234
#define S5P6818_INTERFACE   0
#define S5P6818_EP_OUT      0x02

static libusb_context*         ctx;
static libusb_device_handle*   dev_handle;

int usb_write(const uint8_t* buff, int size)
{
    int transfered;
    int ret;
    ret = libusb_bulk_transfer(dev_handle, S5P6818_EP_OUT, (unsigned char*)buff, size, &transfered, 0);
    if (ret < 0)
        return ret;
    return transfered;
}

int usb_open(void)
{
    libusb_device**         list;
	ssize_t                 cnt;
    int                     ret;

    ret = libusb_init(&ctx);
    if (ret < 0)
        return ret;

    libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_INFO);

    cnt = (int)libusb_get_device_list(ctx, &list);
    if (cnt < 0)
        return cnt;

    dev_handle = libusb_open_device_with_vid_pid(ctx, S5P6818_VID, S5P6818_PID);
    if (dev_handle == NULL)
        return -1;

    libusb_free_device_list(list, 1);
    
    ret = libusb_claim_interface(dev_handle, S5P6818_INTERFACE);
    if (ret < 0)
        return ret;
	
	return 0;
}

void usb_close(void)
{
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
}
