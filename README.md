# mvc

An MPD music visualizer, based on curses. It works by reading the fifo output of an MPD instance. 
It performs a Fast Fourier Transform of the raw PCM data provided by MPD.

## Requirements

No external dependencies other than GCC (or another C99 compatible compiler).

**Optional**: [libmpdclient](https://github.com/MusicPlayerDaemon/libmpdclient) to display current status (song / elapsed time / etc)

The fifo output of MPD needs to be enabled. To do so, simply place this in your mpd.conf:

```
audio_output {
    type                    "fifo"
	name                    "my_fifo"
	path                    "/tmp/mpd.fifo"
	format                  "44100:16:1"
}
```

## Building

Makefile-based.

**Build with status display (requires libmpdclient):**
```
make all
```

**Build without status display (no external dependencies):**
```
make nostatus
```

To install:
```
sudo make install
```

## Usage

Simply run in your terminal:

```
mvc
```

The keypress '**q**'' at anytime **quits** the program and restores the terminal.

Note that an instance of MPD must be running for mvc to work, otherwise the fifo file wouldn't exist.
