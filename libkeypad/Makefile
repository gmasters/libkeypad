# Makefile for libkeypad

CC			= gcc
INCLUDES	= $(wildcard include/*.h)
CFLAGS		= -Wall -g 
LIBS		= 
OBJS		= libkeypad.o

.PHONY: all clean

all: libkeypad.a

libkeypad.a: $(OBJS)
	ar rcs $@ $^
	
%.c.o: %.c
	$(CC) $(CFLAGS) $< -o $@ -c
	
%.c: %.h
	touch $@

clean:
	-@rm -f $(OBJS) libkeypad.a
