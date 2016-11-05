ALL = fft.c utils_curses.c music_visualizer.c 
LOCPATH = ./bin
INSTALLPATH = /usr/local
LFLAGS =  -lm -lcurses
DEBFLAGS = -g -Wall 
NAME = mvc

all:
	mkdir -p $(LOCPATH)
	gcc $(ALL) -o $(LOCPATH)/$(NAME) $(LFLAGS) 
debug:
	mkdir -p $(LOCPATH)/debug
	gcc $(ALL) -o $(LOCPATH)/debug/$(NAME)_debug $(LFLAGS) $(DEBFLAGS)
asan:
	gcc $(ALL) -o $(LOCPATH)/debug/$(NAME)_debug_asan $(LFLAGS) $(DEBFLAGS) -fsanitize=address
clean:
	rm $(LOCPATH)/$(NAME)
	rm $(LOCPATH)/debug/$(NAME)_debug
distclean:
	rm -r $(LOCPATH)
install:
	cp $(LOCPATH)/$(NAME) $(INSTALLPATH)/bin/$(NAME)
uninstall:
	rm $(INSTALLPATH)/bin/$(NAME)



	
