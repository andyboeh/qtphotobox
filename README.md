QtPhotobox
==========

QtPhotobox is a Qt-based C++ application for building a custom Photobox. It is
inspired and heavily based on design (software and look-and-feel) of 
https://github.com/reuterbal/photoobooth with a few
newly added features and some adaptations to my personal use-case.

**Warning:** This is a very, very, *very* quick rewrite in C++ that has not been
tested for stability. Be aware that there may be tons of bugs.

Why a rewrite?
--------------

The original application is written in Python. While Python is a nice language,
it is not the fastest-performing. Plus, I hit some roadblocks when trying
to add new features given the software design. Thus, I decided to rewrite
it in a language I'm currently more fluent.

Features
--------

  * Interface to Cameras via gphoto2
  * Interface to printers via CUPS or an internal Canon Selphy backend (WiFi or USB)
  * GPIO-based LED control (PWM only)
  * Review and reprinting of stored images
  * Possibility to wait for a USB drive and save all files there
  * If a second screen/projectos is attached, slide show on the external screen
  * Flexible positioning/arranging of images on the final print (move, rotate, scale)
  * Custom background for final print
  * Theming support
  * Screensaver during idle time (for advertisements etc.)
  * ...
  
Requirements
------------

The software was developed on a standard x86-64 computer and then tested on
Raspberry Pi 3 and 4 running Raspbian buster. Quite a number of packages need to be installed,
since the software has to be compiled from scratch. 
The software runs on Linux only due to the availability of some libraries.

Installation
------------

You need to compile from source, thus you need development libraries and a build
system for:

  * CMake
  * Qt5
  * Gphoto2
  * libusb
  * CUPS
  * pigpio

Then you can compile from the command line.

Step-by-step:

```
sudo apt-get install build-essential libgphoto2-dev libpigpio-dev cmake libusb-1.0-0-dev qttools5-dev
git clone https://github.com/andyboeh/qtphotobox
mkdir qtphotobox-build
cd qtphotobox-build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../qtphotobox
make
sudo gpasswd -a pi lp
```

The last step is necessary for the integrated Canon Selphy USB backend. It may be skipped
if no Canon printer is attached via USB or you don't want to use the internal Canon
printer driver.

After a successful build, you can run QtPhotobox by typing:

```
~/qtphotobox-build/qtphotobox
```

Configuration
-------------

QtPhotobox can be entirely configured using the GUI. All settings are stored
in a configuration file at `~/.qtphotobox/settings.ini`.

License
-------

As photobooth, the software is available under the AGPL v3.
