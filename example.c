#include <stdio.h>

#include <libkeypad.h>

int main()
{
	int fd;
	enum KeypadPort portA = PORT_A;
	enum KeypadPort portB = PORT_B;
	enum KeypadPort portC = PORT_C;
//	char *keypadname = "/dev/ttyACM0";
	char *keypadname = "dummyfile";
	
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
	
	getchar();
	
	int ok = 1;
	int col;
	int pressed = 0;
	int lastCol = -1;
	enum segChar writeChar;
	// loop over column, always between 0 and 3
	for (col = 0; col < 5 && ok; col++, col %= 4)
	{
		selectColumn(fd, col);
		
		writeChar = getHexRepresentation(col);
		write7seg(fd, writeChar);
		
		enum KeypadButton button = buttonPressed(fd, col);
		//button--;

		if (button == KEY_NONE)
		{
			if (lastCol == col)
			{
				pressed = 0;
			}
			printf("nothing\n");
		}
		else
		{
			if (pressed)
			{
				// ignore button held down
				printf("held\n");
				continue;
			}
			// a new button was pressed
			//~ if (!buttonIsNumeric(button))
				//~ continue;
//~ 
			//~ pincode[count] = getRealNumber(button);
			//~ pressed = 1;
			//~ lastCol = col;
			//~ count++;
			//~ printf("count is %d ", count);
			
			usleep(1000);
		}
	}
	closeKeypad(fd);
	return 0;
}


