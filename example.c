#include <stdio.h>

#include <libkeypad.h>

int main()
{
	int fd;
	char *keypadname = "/dev/ttyACM0";
	
	printf("About to open keypad!\n");
	
	fd = openKeypad(keypadname);
	if (fd < 0)
	{
		fprintf(stderr, "Couldnt open\n");
		return 1;
	}
	closeKeypad(fd);
	return 0;
}
