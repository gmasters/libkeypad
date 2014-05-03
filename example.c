#include <stdio.h>

#include <libkeypad.h>

int main()
{
	int fd;
	enum KeypadPort portA = PORT_A;
	enum KeypadPort portB = PORT_B;
	enum KeypadPort portC = PORT_C;
	char *keypadname = "/dev/ttyACM0";
	
	printf("About to open keypad!\n");
	
	fd = openKeypad(keypadname);
	if (fd < 0)
	{
		fprintf(stderr, "Couldnt open\n");
		return 1;
	}
	
	setPortDirection(fd, portA, DIR_OUT);
	setPortDirection(fd, portB, DIR_IN);
	setPortDirection(fd, portC, DIR_OUT);
	
	closeKeypad(fd);
	return 0;
}
