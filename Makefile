CC	= gcc
INCLUDE	= -I/usr/include -I/usr/local/include
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe -std=gnu99 -O3
LDFLAGS	= -L/usr/lib 
LDLIBS	= -lircclient -lmaxminddb -lcurl
OBJ = geoip.o tidbot.o curl.o cfg.o tell.o tidbit.o manual.o 8ball.o hiscore.o

all: tidbot

tidbot: $(OBJ)
	@echo [link]
	@$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -rf *.o tidbot
