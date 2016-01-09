/*
 * Hidraw Userspace Example
 *
 * Copyright (c) 2010 Alan Ott <alan@signal11.us>
 * Copyright (c) 2010 Signal 11 Software
 *
 * The code may be used by anyone for any purpose,
 * and can serve as a starting point for developing
 * applications using hidraw.
 */

/* Linux */
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include <libudev.h>
/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 */
#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../src/report.h"

const char *bus_str(int bus);

int usage(char **argv)
{
    printf("Usage:\n");
    printf("\t%s <USB device>\n", argv[0]);
    printf("\n");
    printf("The device is likely going to be /dev/hidraw1 or /dev/hidraw2\n");

    return 1;
}

typedef struct _wrapped_report {
	uint8_t report_number;
	USB_JoystickReport_Data_t report;
} __attribute__((packed)) wrapped_report;

int main(int argc, char **argv)
{
	int fd;
	int i, res, desc_size = 0;
	char buf[256];
	struct hidraw_report_descriptor rpt_desc;
	struct hidraw_devinfo info;
    wrapped_report report;

	/* Open the Device with non-blocking reads. In real life,
	   don't use a hard coded path; use libudev instead. */
	if (argc != 2)
	    return usage(argv);

    fd = open(argv[1], O_RDWR);

	if (fd < 0) {
		perror("Unable to open device");
		return 1;
	}

	memset(&rpt_desc, 0x0, sizeof(rpt_desc));
	memset(&info, 0x0, sizeof(info));
	memset(buf, 0x0, sizeof(buf));

	/* Get Report Descriptor Size */
	res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
	if (res < 0)
		perror("HIDIOCGRDESCSIZE");
	else
		printf("Report Descriptor Size: %d\n", desc_size);

	/* Get Report Descriptor */
	rpt_desc.size = desc_size;
	res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
	if (res < 0) {
		perror("HIDIOCGRDESC");

	} else {
		printf("Report Descriptor:\n");
		for (i = 0; i < rpt_desc.size; i++)
			printf("%02x ", rpt_desc.value[i]);
		puts("\n");
	}

	// Get Raw Name
	res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWNAME");
	else
		printf("Raw Name: %s\n", buf);

	 // Get Physical Location 
	res = ioctl(fd, HIDIOCGRAWPHYS(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWPHYS");
	else
		printf("Raw Phys: %s\n", buf);

	 // Get Raw Info 
	res = ioctl(fd, HIDIOCGRAWINFO, &info);
	if (res < 0) {
		perror("HIDIOCGRAWINFO");
	} else {
		printf("Raw Info:\n");
		printf("\tbustype: %d (%s)\n",
			info.bustype, bus_str(info.bustype));
		printf("\tvendor: 0x%04hx\n", info.vendor);
		printf("\tproduct: 0x%04hx\n", info.product);
	}


	 // Get Feature 
	buf[0] = 0x0; // Report Number 
	res = ioctl(fd, HIDIOCGFEATURE(32), buf);
	if (res < 0) {
		perror("HIDIOCGFEATURE");
	} else {
		printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
		printf("Report data (not containing the report number):\n\t");
		for (i = 0; i < res; i++)
			printf("%hhx ", buf[i]);
		puts("\n");
	}

	 // Get Feature 
	buf[0] = 0x0; // Report Number 
	res = ioctl(fd, HIDIOCGFEATURE(32), buf);
	if (res < 0) {
		perror("HIDIOCGFEATURE");
	} else {
		printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
		printf("Report data (not containing the report number):\n\t");
		for (i = 0; i < res; i++)
			printf("%hhx ", buf[i]);
		puts("\n");
	}


    uint8_t bit_count = 0; // one bit set per iteration
    uint8_t raw_count = 0; // increments by 1
    int8_t signed_count = 0;

    // clear the lights
    while(1)
    {
        // Set Feature
        memset(&report, 0, sizeof(report));
        report.report_number = 9;
        report.report.UpdateMask = UPDATE_OTHER_DIRECTION | UPDATE_MAIN_DIRECTION | UPDATE_RADAR_TYPE | UPDATE_MISC | UPDATE_SIGNAL_STRENGTH;
        report.report.MiscDriver = bit_count;
        report.report.OtherDirection = remap_other(degrees_to_value(signed_count));
        report.report.MainDirection = -signed_count;
        report.report.SignalStrength = raw_count;
        report.report.RadarType = raw_count % 8;
        //printf("report size: %d\n", sizeof(report));
        res = ioctl(fd, HIDIOCSFEATURE(sizeof(report)), &report);

        printf("raw_count: %d\n", raw_count);
        
        // sleep for a bit
        usleep(1000*10);
        
        if(bit_count == 0)
        	bit_count = 1;
        else
        	bit_count = bit_count << 1;

        raw_count++;
        signed_count++;
    }

	close(fd);
	return 0;
}

const char *
bus_str(int bus)
{
	switch (bus) {
	case BUS_USB:
		return "USB";
		break;
	case BUS_HIL:
		return "HIL";
		break;
	case BUS_BLUETOOTH:
		return "Bluetooth";
		break;
	case BUS_VIRTUAL:
		return "Virtual";
		break;
	default:
		return "Other";
		break;
	}
}
