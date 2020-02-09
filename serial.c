#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include <gtk/gtk.h>

//replace this with proper device entry later...
char *portdevice = NULL;
int baudrate = 19200;
#define DEFAULTDEV  "/dev/ttyUSB0"

//serial device handle. yes, it's a global
int serialfd = -1;

void open_port(void);
void reopen_port(void);

void reopen_port(void)
{
	if (serialfd != -1)
		close(serialfd);
	open_port();
}

void open_port(void)
    {

	struct termios options;

	if (portdevice==NULL)
	{
		portdevice=malloc(strlen(DEFAULTDEV)+1);
		strcpy(portdevice,DEFAULTDEV);
	}

	serialfd = open(portdevice, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (serialfd == -1)
	{
		fprintf(stderr,"** ERR: Unable to open serial port %s\n",portdevice);
		return;
	}

	fcntl(serialfd, F_SETFL, 0);

	//start fresh
	memset(&options, 0, sizeof(struct termios));

	options.c_cflag = B19200 | CLOCAL | CS8;

	//set baudrate
	switch(baudrate)
	{
		case 19200:
			options.c_cflag = options.c_cflag | B19200;
			break;
		case 9600:
			options.c_cflag = options.c_cflag | B9600;
			break;
		case 4800:
			options.c_cflag = options.c_cflag | B4800;
			break;
		case 2400:
			options.c_cflag = options.c_cflag | B2400;
			break;
		default:
			options.c_cflag = options.c_cflag | B19200;
	}
	options.c_oflag = 0;

	tcflush(serialfd, TCIFLUSH);

	//enable our new settings...
	tcsetattr(serialfd, TCSANOW, &options);
    }

