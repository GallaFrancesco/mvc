SRC = src/settings.h src/fft.c src/utils_curses.c src/music_visualizer.c src/utils_mpd.c src/mt/mt19937ar.c src/beat_track.c src/invaders.c
LOCPATH = ./bin
INSTALLPATH = /usr/local
CFLAGS = -O2
LFLAGS =  -lm -lncurses -ldl -ltinfo #-lasound
DEBFLAGS = -g -Wall
NAME = mvc

LIBMPDCLIENT=$(shell [ -e /usr/include/mpd/client.h ] && echo 0 || echo 1)
ifeq ($(LIBMPDCLIENT), 0)
	LFLAGS_STATUS = -lmpdclient
	CFLAGS += -DSTATUS_CHECK
endif

all:
	mkdir -p $(LOCPATH)
	gcc $(SRC) -o $(LOCPATH)/$(NAME) $(CFLAGS) $(LFLAGS) $(LFLAGS_STATUS)
nostatus:
	mkdir -p $(LOCPATH)
	gcc $(SRC) -o $(LOCPATH)/$(NAME) -O2 $(LFLAGS)
debug:
	mkdir -p $(LOCPATH)/debug
	gcc $(SRC) -o $(LOCPATH)/debug/$(NAME)_debug $(CFLAGS) $(LFLAGS) $(DEBFLAGS) $(LFLAGS_STATUS)
prof:
	mkdir -p $(LOCPATH)/prof
	gcc $(SRC) -o $(LOCPATH)/prof/$(NAME)_prof $(CFLAGS) $(LFLAGS) $(LFLAGS_STATUS) -pg
asan:
	gcc $(SRC) -o $(LOCPATH)/debug/$(NAME)_debug_asan $(CFLAGS) $(LFLAGS) $(DEBFLAGS) $(LFLAGS_STATUS) -fsanitize=address
clean:
	rm -f $(LOCPATH)/$(NAME)
	rm -f $(LOCPATH)/debug/$(NAME)_debug
distclean:
	rm -r $(LOCPATH)
install:
	cp $(LOCPATH)/$(NAME) $(INSTALLPATH)/bin/$(NAME)
uninstall:
	rm $(INSTALLPATH)/bin/$(NAME)
