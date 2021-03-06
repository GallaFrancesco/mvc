# mvc

Minimal MPD music visualizer, based on curses. Works by reading the fifo output of an MPD instance. Now with real-time beat detection and display!

![ExampleBeat](./pics/pic4.png)
![Example1](./pics/pic1.png)
![Example2](./pics/pic2.png)
![Example3](./pics/pic3.png)

## Requirements

**ncurses** and **GCC** (or another C99 compatible compiler).

**Optional**: [libmpdclient](https://github.com/MusicPlayerDaemon/libmpdclient) to display current status (song / elapsed time / etc).

The fifo output of MPD needs to be enabled. To do so, simply place this in your *mpd.conf*:

```
audio_output {
    type                    "fifo"
	name                    "my_fifo"
	path                    "/tmp/mpd.fifo"
	format                  "44100:16:1"
}
```

## Building

**Clone the repo:**
```
git clone https://fragal.eu/git/fra/mvc
cd mvc/
```

**Build with status display (requires libmpdclient):**
```
make all
```
*Note: without libmpdclient installed, this is equivalent to 'make nostatus'*.

**Build without status display (without libmpdclient):**
```
make nostatus
```

Install (to */usr/local/bin/*):
```
sudo make install
```

## Usage

An instance of MPD must be running for mvc to work.

From a console / terminal emulator:
```
mvc
```

### Keybindings

* Quit:                                                 **q**
* Change drawing mode (style):                          **Space bar**
* Move status panel (if built with libmpdclient):       **up / down / left / right keys**
* Reset status panel position:                          **r**
* Toggle status display:                                **t**
* Toggle beat display:                                  **b**
* Increase base frequency of logarithmic bins:          **F**
* Decrease base frequency of logarithmic bins:          **f**
* Decrease octave divider of logarithmic bins:          **O**
* Increase octave divider of logarithmic bins:          **o**
* Print this help:                                      **h**

## Configuration

Editing `settings.h` allows the configuration of:

* Host, Port, Timeout and DB location of the *mpd* instance
* Timeout for the connection to *mpd* to fail
* Number of samples taken each read
* Adaptive sampling on / off: whether to adapt the number of samples to the current sample rate. Might give fairly accurate results with high rates (> 96000 Hz)
* Location of the MPD named pipe (fifo) from which *mvc* reads.
* Seconds in between each status refresh
* Niceness: number of reads to be skipped cyclically (reduces load on the cpu)
* Y\_Correction: integer value to be subtracted to the components in case the height of the
  screen wasn't sufficient
* X\_Correction: lateral shift in case the visualized spectrum is not perfectly aligned with the
  center of the screen.
