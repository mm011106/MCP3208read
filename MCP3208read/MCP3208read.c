/*

 MCP3208 control command for Raspberry Pi
	2013 2 20 v0.1 M.miyamoto

   NAME:
     MCP3208read

  SYNOPSIS:
     MCP3208read [-Dsdmc]

  OPTIONS:
	[-D | --device] device_file	: Specify the device file. /dev/spidev0.0 is the default of this option.
	[-c | --channel] channel	: Select the input channel of AD converter. '1' as the default of this option. See MCP3208 data sheet
	[-s | --speed ] frequency	: apply clock speed in Hz. 500kHz: Default
	[-d | --delay ] time usec	: apply delay in usec. 0us: Default
	[-m | --mode  ] 		: Set Single end mode measurement Diff mode: Default

  NEEDS: SPI interface 



 this command is based on following utility.

*/
/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 *
 * To compile this source, type gcc spidev_test.c -o spidev_test

*/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";

static const int MCP3208_STARTBIT=	0x10;	/*	Always 1 */
static const int MCP3208_SE=		0x08;	/*   Single end mode (bit3='1')  */



// SPI mode must be setup in here!!
static uint8_t mode=SPI_MODE_0;
// 0 : clock normaly Lo & dat on Rising edge, CE active Lo, MSB first
// options for mode setup
/*
#define SPI_CPHA                0x01  Clock Phase
#define SPI_CPOL                0x02  Clock Pol

#define SPI_MODE_0              (0|0)
#define SPI_MODE_1              (0|SPI_CPHA)
#define SPI_MODE_2              (SPI_CPOL|0)
#define SPI_MODE_3              (SPI_CPOL|SPI_CPHA)

#define SPI_CS_HIGH             0x04  CS pol
#define SPI_LSB_FIRST           0x08  bit order
#define SPI_3WIRE               0x10
#define SPI_LOOP                0x20
#define SPI_NO_CS               0x40  CS on/off
#define SPI_READY               0x80
*/


static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
static uint8_t ad_mode=0;
static uint8_t channel=1;

static void transfer(int fd, uint8_t cmd[], uint8_t cmd_size)
{
	int ret;

        uint8_t rx[cmd_size];
        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)cmd,
                .rx_buf = (unsigned long)rx,
                .len = cmd_size,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
        };

//	printf("transfer: size of command= %d \n",cmd_size);

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)

		pabort("can't send spi message");


	for (ret = 1; ret < cmd_size ; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X", rx[ret]);
	}

	puts("");


}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-Dsdsc]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -m --mode     Single End mode\n"
	     "  -c --channel    channel number \n"
	);
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "mode",    0, 0, 'm' },
			{ "channel", 1, 0, 'c' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:mc:", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'm':
			ad_mode=1;
			break;
		case 'c':
			channel=atoi(optarg);
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	uint8_t commands[]={0x11,0x22,0XFF};

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

//	printf("spi mode: %d\n", mode);
//	printf("bits per word: %d\n", bits);
//	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
//
//	printf("AD mode=%d, channel=%d\n",ad_mode,channel);
//	printf("channel=%d \n",channel);
//	printf("mode =%d \n",MCP3208_SE*ad_mode);
//	printf("command =0x%x \n",MCP3208_STARTBIT | MCP3208_SE*(ad_mode)|channel);

	// put the command code into two bytes to fit hole transmission into 24 bit(8x3)
	// See MCP3208 datasheet
	//  xxxx x1..   ..XX XXXX   XXXX XXXX
	//        |||   ||
	//        ||2   10 Channel select(3bits)
	//        ||
	//	  |SE/!DIFF mode
	//        StartBit
	channel=channel & 0x07;
	commands[0]=(MCP3208_STARTBIT | MCP3208_SE*(ad_mode & 1) | channel)/4;
	commands[1]=(channel*64) & 0xFF;

	transfer(fd, commands, ARRAY_SIZE(commands));

	close(fd);

	return ret;
}

