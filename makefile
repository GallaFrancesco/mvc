SRC = src/settings.h src/fft.c src/utils_curses.c src/music_visualizer.c src/utils_mpd.c
ALL = $(SRC)
LOCPATH = ./bin
INSTALLPATH = /usr/local
CFLAGS = -O2
LFLAGS =  -lm -lcurses -ldl #-ltinfo #-lasound
DEBFLAGS = -g -Wall
NAME = mvc

LIBMPDCLIENT=$(shell [ -e /usr/include/mpd/client.h ] && echo 0 || echo 1)
ifeq ($(LIBMPDCLIENT), 0)
	LFLAGS_STATUS = -lmpdclient
	CFLAGS += -DSTATUS_CHECK
endif

all:
	mkdir -p $(LOCPATH)
	gcc $(ALL) -o $(LOCPATH)/$(NAME) $(CFLAGS) $(LFLAGS) $(LFLAGS_STATUS)
nostatus:
	mkdir -p $(LOCPATH)
	gcc $(ALL) -o $(LOCPATH)/$(NAME) -O2 $(LFLAGS)
debug:
	mkdir -p $(LOCPATH)/debug
	gcc $(ALL) -o $(LOCPATH)/debug/$(NAME)_debug $(CFLAGS) $(LFLAGS) $(DEBFLAGS) $(LFLAGS_STATUS)
asan:
	gcc $(ALL) -o $(LOCPATH)/debug/$(NAME)_debug_asan $(CFLAGS) $(LFLAGS) $(DEBFLAGS) -fsanitize=address
clean:
	rm $(LOCPATH)/$(NAME)
	rm $(LOCPATH)/debug/$(NAME)_debug
distclean:
	rm -r $(LOCPATH)
install:
	cp $(LOCPATH)/$(NAME) $(INSTALLPATH)/bin/$(NAME)
uninstall:
	rm $(INSTALLPATH)/bin/$(NAME)
