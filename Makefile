CC	= gcc
INCLUDE	= -I/usr/include -I/usr/local/include
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe -std=gnu99 -O3
LDFLAGS	= -L/usr/lib 
LDLIBS	= -lircclient -lmaxminddb
OBJ = geoip.o tidbot.o

all: tidbot

tidbot: $(OBJ)
	@echo [link]
	@$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -rf *.o tidbot
