# mvc

An MPD music visualizer, based on curses. It works by reading the fifo output of an MPD instance. 
At the moment it performs a Fast Fourier Transform of the raw PCM data provided by MPD, but a beat detection is going to be implemented to gain more precision.

## Requirements

The fifo output of MPD needs to be enabled. To do so, simply place this in your mpd.conf:

```
audio_output {
    type                    "fifo"
	name                    "my_fifo"
	path                    "/tmp/mpd.fifo"
	format                  "44100:16:1"
}
```
This is allowing MPD to create a file (the location is hardcoded right now so it must be /tmp/mpd.fifo) and write it with raw PCM data in realtime.
Path and format variables MUST be the same displayed above.
By reading that, mvc is able to compute a frequency spectrum of the current track played.

## Building

Makefile-based.  
```
make all
```

To install:
```
sudo make install
```

## Usage

Simply run:

```
mvc
```

Note that an instance of MPD must be running for mvc to work, otherwise the fifo file doesn't exist.

