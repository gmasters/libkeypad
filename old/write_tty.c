// most from http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

enum segChar {ZERO=0x3F, ONE=0x06, TWO=0x5B, THREE=0x4F, FOUR=0x66, FIVE=0x6D,
	      SIX=0x7D, SEVEN=0x07, EIGHT=0x7F, NINE=0x6F, BLAH=0x77};

//{0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x58,0x5E,0x79,0x71,0x40,0xD3};

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

int main()
{
	char *portname = "/dev/ttyACM0";
	char* input = NULL;
	int i;
	int seg = ZERO;

	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
		fprintf (stderr, "error %d opening %s: %s\n", errno, portname, strerror (errno));
		return;
	}

	set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	printf("set up\n");

	write (fd, "@00D200\r", 8);
	char buf [100];
	int n = read (fd, buf, sizeof buf);

	write (fd, "@00D000\r", 8);
	n = read (fd, buf, sizeof buf);

	char pints[10];
	for (i=0;i<0x10;i++)
	{
		printf("writing to led %x\n", i);
		sprintf (pints, "@00P00%x\r", i);
		printf("%s\n", pints);
		write (fd, pints, 8);
		usleep(1000);

		sprintf (pints, "@00P2%x\r", seg);
		write (fd, "@00P23F\r", 8);

		usleep(1000000);


	}
}
