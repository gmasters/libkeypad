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
	//	set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	//	set_blocking (fd, 0);                // set no blocking
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
	printf("buf %d %x\n", yes, buf[0]);

	if (buf[0] != '!')
	{
		fprintf(stderr, "Bad Response\n");
	}
	else if (strcmp(buf,"!0000\r") != 0)
	{
		// something was pressed
		int row = getRowNumber(&buf[4]);
		button = getButton(row, col);
	}
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

enum segChar getHexRepresentation(int character)
{
	enum segChar seg;
	switch (character)
	{
	case 1:
		seg = SEG_ONE; break;
	case 2:
		seg = SEG_TWO; break;
	case 3:
		seg = SEG_THREE; break;
	case 4:
		seg = SEG_FOUR; break;
	case 5:
		seg = SEG_FIVE; break;
	case 6:
		seg = SEG_SIX; break;
	case 7:
		seg = SEG_SEVEN; break;
	case 8:
		seg = SEG_EIGHT; break;
	case 9:
		seg = SEG_NINE; break;
	case 0:
		seg = SEG_ZERO; break;
	case 'A':
		seg = SEG_A; break;
	case 'B':
		seg = SEG_B; break;
	case 'C':
		seg = SEG_C; break;
	case 'D':
		seg = SEG_D; break;
	case 'E':
		seg = SEG_E; break;
	case 'F':
		seg = SEG_F; break;
	default: // BLANK
		seg = SEG_BLANK;
	}
	return seg;
}

//~ 
//~ void selectCol(int col)
//~ {
	//~ char buf [100];
	//~ int n;
	//~ if (col == 0)
	//~ {
		//~ write (fd, "@00P001\r", 8);
		//~ n = read (fd, buf, sizeof buf);
		//~ if (!n)
			//~ fprintf(stderr, "Read failed\n");
	//~ }
	//~ else if (col == 1)
	//~ {
		//~ write (fd, "@00P002\r", 8);
		//~ n = read (fd, buf, sizeof buf);
		//~ if (!n)
			//~ fprintf(stderr, "Read failed\n");
	//~ }
	//~ else if (col == 2)
	//~ {
		//~ write (fd, "@00P004\r", 8);
		//~ n = read (fd, buf, sizeof buf);
		//~ if (!n)
			//~ fprintf(stderr, "Read failed\n");
	//~ }
	//~ else if (col == 3)
	//~ {
		//~ write (fd, "@00P008\r", 8);
		//~ n = read (fd, buf, sizeof buf);
		//~ if (!n)
			//~ fprintf(stderr, "Read failed\n");
	//~ }
//~ 
	//~ usleep(1000);
//~ }
//~ 
//~ void writeNum(int number)
//~ {
	//~ enum segChar writeChar;
	//~ char buf [100];
	//~ char pints[10];
	//~ int n;
	//~ printf("num %d\n", number);
	//~ if (number < 0)
	//~ {
		//~ writeChar = 0x00;
	//~ }
	//~ else
	//~ {
 		//~ writeChar = getHexRepresentation(number); 
	//~ }
//~ 
	//~ sprintf (pints, "@00P2%02x\r", writeChar);
//~ //	printf("Displaying: %02x\n", writeChar);
//~ //	printf("PINTS %s\n", pints);
	//~ write (fd, pints, 8);
	//~ n = read (fd, buf, sizeof buf);
	//~ if (!n)
		//~ fprintf(stderr, "Read failed\n");
	//~ usleep(1000);
//~ 
	//~ // write blank so it doesnt interfere with the next
	//~ write (fd, "@00P200\r", 8);
	//~ n = read (fd, buf, sizeof buf);
	//~ if (!n)
		//~ fprintf(stderr, "Read failed\n");
	//~ usleep(50);
//~ }
//~ 
//~ enum KeypadButton buttonPressed(int col)
//~ {
	//~ char buf [100];
	//~ int n;
//~ 
	//~ write (fd, "@00P1?\r", 7);
	//~ n = read (fd, buf, sizeof buf);
	//~ if (!n)
		//~ fprintf(stderr, "Read failed\n");
	//~ usleep(100);
//~ 
	//~ if (strcmp(buf,"!0000\r") != 0)
	//~ {
		//~ //getchar();
		//~ // something was pressed
		//~ int row = 0;
		//~ enum KeypadButton button = KEY_1;
		//~ 
		//~ row = getRowNumber(&buf[4]);
		//~ return getButton(row,col);
	//~ }
	//~ return KEY_NONE;
//~ }
//~ 
//~ 
//~ int getRowNumber(char* ch)
//~ {
	//~ int row = 0;
	//~ long int buttonValueRead;
	//~ //last digit will be 1,2,4 or 8
	//~ /*switch (buf[4])
	//~ {
	//~ case '1': // number could be 1 2 3 F
		//~ row = 1; break;
	//~ case '2': // number could be 4 5 6 E
		//~ row = 2; break;
	//~ case '4': // number could be 7 8 9 D
		//~ row = 3; break;
	//~ default: // number could be A 0 B C	
		//~ row = 4; break;
	//~ }*/
	//~ // printf("buf4 %c\trow: %d\n", ch], row);
//~ 
	//~ buttonValueRead = strtol(ch, NULL, 16);
	//~ printf("hex: %x, %li\n", buttonValueRead, buttonValueRead);
	//~ if (buttonValueRead & 0x1)
	//~ {
		//~ row = 1;
	//~ }
	//~ else if (buttonValueRead & 0x2)
	//~ {
		//~ row = 2;
	//~ }			
	//~ else if (buttonValueRead & 0x4)
	//~ {
		//~ row = 3;
	//~ }			
	//~ else /*(buttonValueRead & 0x8)*/
	//~ {
		//~ row = 4;
	//~ }
	//~ return row;
//~ }
//~ 
//~ // return 1 if a button pressed else 0
//~ int checkColumnForPressed(int column)
//~ {
	//~ char pints[10];
	//~ int colHex;
	//~ int n=0;
	//~ char buf [100];
//~ 
	//~ switch (column)
	//~ {
	//~ case 1:
		//~ colHex = 1; break;
	//~ case 2:
		//~ colHex = 2; break;
	//~ case 3:
		//~ colHex = 4; break;
	//~ default:
		//~ colHex = 8; break;
	//~ }
	//~ printf("column: %d\n", column);
//~ 
	//~ // set port0 to only read 1 column
	//~ sprintf (pints, "@00P00%1x\r", colHex);
	//~ write (fd, pints, 8);
	//~ n = read (fd, buf, sizeof buf);
	//~ if (!n)
		//~ fprintf(stderr, "Read failed\n");
	//~ usleep(10);
//~ 
	//~ // now do the actual read on port1
	//~ write (fd, "@00P1?\r", 7);
	//~ n = read (fd, buf, sizeof buf);
	//~ if (!n)
		//~ fprintf(stderr, "Read failed\n");
	//~ usleep(100);
//~ 
	//~ if (strcmp(buf,"!0000\r") != 0)
	//~ {
		//~ return 1;
	//~ }
	//~ return 0;
//~ }
//~ 
//~ enum KeypadButton getButton(int row, int col)
//~ {
	//~ return ((row - 1) * 4) + (col + 1);
//~ }
//~ 
//~ // return 1 = it is
//~ int buttonIsNumeric(enum KeypadButton button)
//~ {
	//~ int isNumeric;
	//~ switch (button)
	//~ {
	//~ case KEY_A:
	//~ case KEY_B:
	//~ case KEY_C:
	//~ case KEY_D:
	//~ case KEY_E:
	//~ case KEY_F:
		//~ isNumeric = 0; break;
	//~ default /*Numeric*/:
		//~ isNumeric = 1; break;
	//~ }
	//~ return isNumeric;
//~ }
//~ 
//~ int getRealNumber(enum KeypadButton button)
//~ {
	//~ int number;
	//~ switch (button)
	//~ {
	//~ case KEY_1:
		//~ number = 1; break;
	//~ case KEY_2:
		//~ number = 2; break;
	//~ case KEY_3:
		//~ number = 3; break;
	//~ case KEY_4:
		//~ number = 4; break;
	//~ case KEY_5:
		//~ number = 5; break;
	//~ case KEY_6:
		//~ number = 6; break;
	//~ case KEY_7:
		//~ number = 7; break;
	//~ case KEY_8:
		//~ number = 8; break;
	//~ case KEY_9:
		//~ number = 9; break;
	//~ case KEY_0:
		//~ number = 0; break;
	//~ default /*Letter*/:
		//~ number = -1; break;
	//~ }
	//~ return number;
//~ }
//~ 
//~ void showButton(enum KeypadButton button)
//~ {
	//~ char buf [100];
	//~ char pints[10];
//~ //	char buttonChar = buttonStrings[button][0];
	//~ enum segChar hexToPrint = getHexRepresentation(button);
	//~ sprintf (pints, "@00P2%02x\r", hexToPrint);
	//~ printf("Displaying: %02x\n", hexToPrint);
	//~ printf("PINTS %s\n", pints);
	//~ write (fd, pints, 8);
	//~ read (fd, buf, sizeof buf);
	//~ usleep(1000);
//~ }
//~ 
//~ enum segChar getHexRepresentation(int button)
//~ {
	//~ enum segChar seg;
	//~ //button++;
	//~ switch (button)
	//~ {
	//~ case 1:
		//~ seg = ONE; break;
	//~ case 2:
		//~ seg = TWO; break;
	//~ case 3:
		//~ seg = THREE; break;
//~ /*	case KEY_F:
		//~ seg = F; break;*/
	//~ case 4:
		//~ seg = FOUR; break;
	//~ case 5:
		//~ seg = FIVE; break;
	//~ case 6:
		//~ seg = SIX; break;
//~ /*	case KEY_E:
		//~ seg = E; break;*/
	//~ case 7:
		//~ seg = SEVEN; break;
	//~ case 8:
		//~ seg = EIGHT; break;
	//~ case 9:
		//~ seg = NINE; break;
//~ /*	case KEY_D:
		//~ seg = D; break;
	//~ case KEY_A:
		//~ seg = A; break;*/
	//~ case 0:
		//~ seg = ZERO; break;
//~ /*	case KEY_B:
		//~ seg = B; break;
	//~ default:
		//~ seg = C; break;*/
	//~ }
	//~ return seg;
//~ }
//~ 
