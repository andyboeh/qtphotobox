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
  * Interface to printers via CUPS or an internal Canon Selphy backend (WiFi only)
  * GPIO-based LED control (PWM only)
  * Review and reprinting of stored images
  * Possibility to wait for a USB drive and save all files there
  * If a second screen/projectos is attached, slide show on the external screen
  * Flexible positioning/arranging of images on the final print (move, rotate, scale)
  * Custom background for final print
  * Theming support
  
Requirements
------------

The software was developed on a standard x86 computer and then tested on a
Raspberry Pi 4 using Raspbian buster. Quite a number of packages need to be installed,
since the software has to be compiled from scratch. 
The software runs on Linux only due to the availability of some libraries.

Installation
------------

WiP

Configuration
-------------

The GUI settings screen is still WiP. You can customize the settings by modifying
the file ~/.qtphotobox/settings.ini with your favorite text editor.

License
-------

As photobooth, the software is available under the AGPL v3.
