CC	= gcc
INCLUDE	= -I/usr/include -I/usr/local/include
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe -std=gnu99 -O3
LDFLAGS	= -L/usr/lib 
LDLIBS	= -lircclient -lmaxminddb

all: tidbot

tidbot: tidbot.o geoip.o
	@echo [link]
	@$(CC) -o $@ tidbot.o geoip.o $(LDFLAGS) $(LDLIBS)

geoip: geoip.o
	@$(CC) -o $@ geoip.o $(LDFLAGS) $(LDLIBS)

clean:
	rm *.o
