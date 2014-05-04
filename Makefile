# Makefile for application using libkeypad

CC			= gcc
INCLUDES	= libkeypad
CFLAGS		= -Wall -g -I$(INCLUDES)
#$(INCLUDES)
LIBS		= $(INCLUDES)/libkeypad.a
OBJS		= example.o

.PHONY: all clean

all: example

example: $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	
%.c.o: %.c
	$(CC) $(CFLAGS) $< -o $@ -c

clean:
	-@rm -f $(OBJS) example
