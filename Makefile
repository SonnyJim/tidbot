CC	= gcc
INCLUDE	= -I/usr/include
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe -std=c99 -O3
LDFLAGS	= -L/usr/lib 
LDLIBS	= -lircclient

all: tidbot

tidbot: tidbot.o
	@echo [link]
	@$(CC) -o $@ tidbot.o $(LDFLAGS) $(LDLIBS)

clean:
	rm *.o
