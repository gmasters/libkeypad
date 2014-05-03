#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

enum KeypadButton{KEY_NONE, 
		  		  KEY_1, KEY_2, KEY_3, KEY_F,
                  KEY_4, KEY_5, KEY_6, KEY_E,
                  KEY_7, KEY_8, KEY_9, KEY_D,
                  KEY_A, KEY_0, KEY_B, KEY_C};

enum segChar {ZERO=0x3F, ONE=0x06, TWO=0x5B, THREE=0x4F, FOUR=0x66, FIVE=0x6D,
	      SIX=0x7D, SEVEN=0x07, EIGHT=0x7F, NINE=0x6F, A=0x77, B=0x7C,
	      C=0x39, D=0x5E, E=0x79, F=0x71};
	      
enum KeypadPort {PORT_A, PORT_B, PORT_C};

enum PortDirection {DIR_IN, DIR_OUT};

/* opens the keypad
 * on success returns file descriptor for keypad
 * on failure returns < 0
 */
int openKeypad(char* keypadLocation);

int closeKeypad(int fd);

int setPortDirection(int fd, enum KeypadPort portNumber, enum PortDirection direction); 


//~ int getRowNumber(char*);
//~ int checkColumnForPressed(int);
//~ enum KeypadButton getButton(int, int);
//~ void showButton(enum KeypadButton);
//~ enum segChar getHexRepresentation(int button);
//~ int buttonIsNumeric(enum KeypadButton);
//~ int getRealNumber(enum KeypadButton button);
//~ 
//~ void selectCol(int col);
//~ void writeNum(int number);
//~ enum KeypadButton buttonPressed(int col);
