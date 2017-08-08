#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "usbdeal.h"

#define VERSION             "1.0.0"

#define FILE_MAX_SIZE       (56 << 10)                                  // 56KB

static int8_t buff[512];
static int timeout = 10;
static bool kernel=false;

char *const short_options = "hvt:k";
struct option long_options[] = {
    {"help", 0, NULL, 'h'},
    {"version", 0, NULL, 'v'},
	{"time", 0, NULL, 't'},
	{"kernel", 0, NULL, 'k'},
    {0, 0, 0, 0}};

void usage(const char* appName)
{
    printf("Usage: %s <filepath>\n", appName);
    printf("Download firmware to s5p6818 from usb.\n");
    printf("Mandatory arguments to long options are mandatory for short options too.\n");
    printf("\n");
    printf("-h, --help              display this help and exit\n");
    printf("-v, --version           output version information and exit\n");
	printf("-t, --time              timeout for search driver, default timeout is 10 second\n");
	printf("-k, --kernel            download kernel, default is u-boot whit nsih header\n");
}

void version(const char* appName)
{
    printf("%s %s\n", appName, VERSION);
}

unsigned long getFileSize(const char* filePath)
{
    unsigned long fileSize = -1;
    struct stat statbuff;
    if (stat(filePath, &statbuff) < 0)
        return fileSize;
    else
        fileSize = statbuff.st_size;
    return fileSize;
}

int main(int argc, char *argv[])
{
    int ret;
	int i;
	
    while ((ret = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch (ret)
        {
		case 't':
			timeout = atoi(optarg);
			break;
		case 'k':
			kernel = true;
			break;
        case 'v':
            version(argv[0]);
            return 0;
        case 'h':
        case '?':
            usage(argv[0]);
            return 0;
        }
    }

    char *filePath;
    if (optind < argc)
        filePath = argv[optind];
    else
        usage(argv[0]);

    int numRead, numWrite;
    int fd;
    fd = ret = open(filePath, O_RDONLY);
    if (ret < 0)
        return ret;

    unsigned long fileSize = getFileSize(filePath);
	
	if (!kernel)
	{
		if(fileSize > FILE_MAX_SIZE)
		{
			fprintf(stderr, "[Error] Firmware is greater than 56KB.\n");
			return -1;
		}
		
		if(fileSize < 0x200)
		{
			fprintf(stderr, "[Error] Nish header no correct.\n");
			return -1;
		}

		if((numRead = read(fd, buff, 0x200)) != 0x200)
		{
			fprintf(stderr, "[Error] Read nish header failed.\n");
			return -1;
		}		
	}

    printf("[Info] Start to transfer the firmwave...\n");

	for(i = 0; i < timeout * 10; i++)
	{
		if(!usb_open())
		{
			break;
		}
		if(i % 5 == 0)
		{
			printf(".");
			fflush(stdout);
		}
		usleep(100000);
	}
	printf("\n");
	if(i == timeout * 10)
	{
		fprintf(stderr, "[Error] Can't find the S5P6818 driver.\n");
		return -1;
	}

    if(!kernel)
		*((unsigned int*)(buff + 0x44)) = fileSize - 0x200;                 // Change LOADSIZE
	else
		*((unsigned int*)(buff + 0x44)) = fileSize;                         // Change LOADSIZE

    if((numWrite = usb_write(buff, 0x200)) != 0x200)
    {
        fprintf(stderr, "[Error] Transfer nish header failed.\n");
        usb_close();
        return -1;
    }

    while(1)
    {
        if((numRead = read(fd, buff, 0x200)) == -1)
        {
            fprintf(stderr, "[Error] Read firmwave failed.\n");
            usb_close();
            return -1;
        }

        if((numWrite = usb_write(buff, numRead)) != numRead)
        {
            fprintf(stderr, "[Error] Transfer firmwave failed.\n");
            usb_close();
            return -1;
        }        

        if(numWrite < 0x200)
            break;
    }

    ret = close(fd);
    if (ret < 0)
    {
        usb_close();
        return ret;
    }

    usb_close();

    printf("[Info] Success to transfer the firmwave.\n");
    return 0;
}
