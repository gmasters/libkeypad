/**
@file :	
	libkeypad.c

@version : 
	1.0

@authors : 
	Gregory Masters
	Thomas West
	Steven Square
	Nathan Toft

@description :
	Source file for keypad library
	This file contains the implementation of functionality for the keypad library
**/

#include "libkeypad.h"

#define BUFMAX 100

int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);

int openKeypad(char* keypadLocation)
{
	int fd = open (keypadLocation, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
		fprintf (stderr, "Error %d opening %s: %s\n", errno, keypadLocation, strerror (errno));
	}
	else
	{
		// opened ok, now set it up
		set_interface_attribs (fd, B115200, 0); // set speed to 115,200 bps, 8n1 (no parity)
		set_blocking (fd, 0); // set no blocking
	}
	return fd;
}

int closeKeypad(int fd)
{
	if (fd < 0)
	{
		fprintf (stderr, "Can't close %d, not open\n", fd);
		return -1;
	}
	return close(fd);
}

int writeKeypad(int fd, char* message, int size)
{
	int nbytes;
	int nbytesTotal = 0;
	char* unwritten;
	do
	{
		// set unwritten pointer to last written char
		unwritten = &message[nbytesTotal];
		
		// do write
		nbytes = write(fd, unwritten, size);
		if (nbytes < 0)
		{
			fprintf (stderr, "Error while writing\n");
			return -1;
		}
		nbytesTotal += nbytes;
	} while (nbytesTotal < size);
	return nbytesTotal;
}

int readKeypad(int fd, char* buf, int size)
{
	return read(fd, buf, size);
}

int sendToKeypad(int fd, char* message, char* response)
{
	int ret = writeKeypad(fd, message, strlen(message));
	printf("written %d\n", ret);
	if (ret < 0)
	{
		return ret;
	}
	return readKeypad(fd, response, BUFMAX);
}

int setPortDirection(int fd, enum KeypadPort portNumber, enum PortDirection direction)
{
	char message[10];
	char buf[BUFMAX];
	char port;
	int dir;
	
	// assign char for port
	if (portNumber == PORT_A)
	{
		port = '0';
	}
	else if (portNumber == PORT_B)
	{
		port = '1';
	}
	else // (portNumber == PORT_C)
	{
		port = '2';
	}
	
	// assign char for direction
	if (direction == DIR_IN)
	{
		dir = 0xFF;
	}
	else // (direction == DIR_OUT)
	{
		dir = 0x00;
	}
	
	// construct message
	sprintf(message, "@00D%c%02x\r", port, dir);
	int yes = sendToKeypad(fd, message, buf);
	printf("buf %d %s\n", yes, buf);
//	printf("message %s\n", message);
//	getchar();
	usleep(50);
	return 0;
}

int selectColumn(int fd, int col)
{
	char colChar;
	char message[10];
	int ret = 0;
	char buf[BUFMAX];
	
	switch (col)
	{
	case 0: 
		colChar = '1';
		break;
	case 1: 
		colChar = '2';
		break;
	case 2: 
		colChar = '4';
		break;
	case 3: 
		colChar = '8';
		break;
	default: 
		fprintf(stderr, "Invalid Column\n");
		ret = -1;
		colChar = 0;
	}
	
	if (colChar)
	{
		// construct message
		sprintf(message, "@00P00%c\r", colChar);	
		int yes = sendToKeypad(fd, message, buf);
		printf("buf %d %x\n", yes, buf[0]);
		//~ printf("message %s\n", message);
		//~ getchar();
	}

	usleep(50);
	return ret;
}

int write7seg(int fd, enum segChar character)
{
	char message[10];
	char buf[BUFMAX];
	// construct message
	sprintf(message, "@00P2%02x\r", character);	
	int yes = sendToKeypad(fd, message, buf);
	printf("buf %d %x\n", yes, buf[0]);
	return 0;
}

enum KeypadButton buttonPressed(int fd, int col)
{
	enum KeypadButton button = KEY_NONE;
	char buf[BUFMAX];

	int yes = sendToKeypad(fd, "@00P1?\r", buf);
//	printf("buf %d %s\n", yes, buf);

	if (buf[0] != '!')
	{
		fprintf(stderr, "Bad Response\n");
	}
	else if (strncmp(buf,"!0000", 5) != 0)
	{
		// something was pressed
		int row = getRowNumber(&buf[4]);
		button = getButton(row, col);
	}
	//~ printf("message %c%c%c%c%c\n", buf[0],buf[1],buf[2],buf[3],buf[4]);
	//~ getchar();
	return button;
}

int getRowNumber(char* ch)
{
	int row = 0;
	long int buttonValueRead;

	buttonValueRead = strtol(ch, NULL, 16);
	printf("hex: %lx, %li\n", buttonValueRead, buttonValueRead);
	if (buttonValueRead & 0x1)
	{
		row = 1;
	}
	else if (buttonValueRead & 0x2)
	{
		row = 2;
	}			
	else if (buttonValueRead & 0x4)
	{
		row = 3;
	}			
	else /*(buttonValueRead & 0x8)*/
	{
		row = 4;
	}
	return row;
}

enum KeypadButton getButton(int row, int col)
{
	return ((row - 1) * 4) + (col + 1);
}

// rubbish to set it up - move to seperate file
int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                fprintf (stderr, "error %d from tcgetattr\n", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // ignore break signal
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                fprintf (stderr, "error %d from tcsetattr\n", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                fprintf (stderr, "error %d from tggetattr\n", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                fprintf (stderr, "error %d setting term attributes\n", errno);
}

enum segChar getHexRepresentation(enum KeypadButton button)
{
	enum segChar seg;
	switch (button)
	{
	case KEY_1:
		seg = SEG_ONE; break;
	case KEY_2:
		seg = SEG_TWO; break;
	case KEY_3:
		seg = SEG_THREE; break;
	case KEY_4:
		seg = SEG_FOUR; break;
	case KEY_5:
		seg = SEG_FIVE; break;
	case KEY_6:
		seg = SEG_SIX; break;
	case KEY_7:
		seg = SEG_SEVEN; break;
	case KEY_8:
		seg = SEG_EIGHT; break;
	case KEY_9:
		seg = SEG_NINE; break;
	case KEY_0:
		seg = SEG_ZERO; break;
	case KEY_A:
		seg = SEG_A; break;
	case KEY_B:
		seg = SEG_B; break;
	case KEY_C:
		seg = SEG_C; break;
	case KEY_D:
		seg = SEG_D; break;
	case KEY_E:
		seg = SEG_E; break;
	case KEY_F:
		seg = SEG_F; break;
	default: // KEY_NONE
		seg = SEG_BLANK;
	}
	return seg;
}

int buttonIsNumeric(enum KeypadButton button)
{
	int isNumeric;
	switch (button)
	{
	case KEY_NONE:
		isNumeric = -1; break;
	case KEY_A:
	case KEY_B:
	case KEY_C:
	case KEY_D:
	case KEY_E:
	case KEY_F:
		isNumeric = 0; break;
	default /*Numeric*/:
		isNumeric = 1; break;
	}
	return isNumeric;
}

int getRealNumber(enum KeypadButton button)
{
	int number;
	switch (button)
	{
	case KEY_1:
		number = 1; break;
	case KEY_2:
		number = 2; break;
	case KEY_3:
		number = 3; break;
	case KEY_4:
		number = 4; break;
	case KEY_5:
		number = 5; break;
	case KEY_6:
		number = 6; break;
	case KEY_7:
		number = 7; break;
	case KEY_8:
		number = 8; break;
	case KEY_9:
		number = 9; break;
	case KEY_0:
		number = 0; break;
	default /*Letter or blank*/:
		number = -1; break;
	}
	return number;
}
