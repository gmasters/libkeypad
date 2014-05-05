/**
@file :	
	libkeypad.h

@version : 
	1.0

@authors : 
	Gregory Masters
	Thomas West
	Steven Square
	Nathan Toft

@description :
	Header file for libkeypad.c
	This file contains the includes, defined types and function declarations
**/

// Includes
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// The buttons on the keypad
enum KeypadButton 
{
	KEY_NONE, 
	KEY_1, KEY_2, KEY_3, KEY_F,
	KEY_4, KEY_5, KEY_6, KEY_E,
	KEY_7, KEY_8, KEY_9, KEY_D,
	KEY_A, KEY_0, KEY_B, KEY_C
};

// Hexadecimal value to display the character correctly
enum segChar 
{	
	SEG_BLANK=0x00, SEG_ZERO=0x3F, SEG_ONE=0x06, SEG_TWO=0x5B, SEG_THREE=0x4F, 
	SEG_FOUR=0x66, SEG_FIVE=0x6D, SEG_SIX=0x7D, SEG_SEVEN=0x07, SEG_EIGHT=0x7F, 
	SEG_NINE=0x6F, SEG_A=0x77, SEG_B=0x7C, SEG_C=0x39, SEG_D=0x5E, SEG_E=0x79, SEG_F=0x71
};

// The ports on the keypad
enum KeypadPort { PORT_A, PORT_B, PORT_C };

// The directions for the port/s on the keypad
enum PortDirection { DIR_IN, DIR_OUT };

/**
 Open the keypad for reading or writing
 
 @param keypadLocation: The path to the location of the keypad
 @return  
	success: a file descriptor for the keypad
	failure: an integer < 0
*/
int openKeypad(char* keypadLocation);

/**
 Close the keypad
 
 @param fd: The file descriptor for the keypad
 @return : Whether the close operation was successful or not
*/
int closeKeypad(int fd);

/**
 Write a message to the keypad
 
 @param fd: The file descriptor for the keypad
 @param message: The message to write to the keypad
 @param size: The length of the message
 @return : Whether the write was a success
*/
int writeKeypad(int fd, char* message, int size);

/**
 Read from the keypad and put data into a buffer
 
 @param fd: The file descriptor for the keypad - where to read from
 @param buf: The buffer to put the read data into
 @param size: The number of bytes to be read
 @return 
	success: number of bytes read
	failure: -1
*/
int readKeypad(int fd, char* buf, int size);

/**
 Set a port on the keypad to be an input port or output port
 
 @param fd: The file descriptor for the keypad
 @param portNumber: The port that requires setup
 @param direction: The direction required - input or output
 @return : Whether the setup was a success
*/
int setPortDirection(int fd, enum KeypadPort portNumber, enum PortDirection direction);

/**
 Select a column on the keypad so that it can be read from
 
 @param fd: The file descriptor for the keypad
 @param col: The column desired
 @return : Whether the selection was a success
*/
int selectColumn(int fd, int col);

/**
 Get the hexadecimel value required to show a character
 
 @param character: The character for which hexadecimal representation is required
 @return : The enum value that contains the desired hexadecimal value
*/
enum segChar getHexRepresentation(enum KeypadButton button);

/**
 Write a value to the 7 segment display on the keypad
 
 @param fd: The file descriptor for the keypad
 @param character: The hexadecimel value required to show the desired character
 @return : Whether the write was a success
*/
int write7seg(int fd, enum segChar character);

/**
 Detect what button has been pressed
 
 @param fd: The file descriptor for the keypad
 @param col: The column that is setup to be read from
 @return : The enum value for the button that has been pressed
*/
enum KeypadButton buttonPressed(int fd, int col);

/**
 Get an integer representation of a row number
 
 @param ch: The character containing the row valueto be decrypted
 @return : The integer representation for the row (1->4)
*/
int getRowNumber(char* ch);

/**
 Get the button that has been pressed from a row, column combination
 
 @param row: The row number of the button
 @param col: The column number of the button
 @return : The button that is on the row/col specified
*/
enum KeypadButton getButton(int row, int col);

/**
 Check if the button is 0-9
 
 @param button: The button to check
 @return : True/False, whether the button is a number or not
*/
int buttonIsNumeric(enum KeypadButton button);

/**
 Gives an integer representation of a numeric button
 
 @param button: The numeric button
 @return : The integer representation of the numeric button
*/
int getRealNumber(enum KeypadButton button);
