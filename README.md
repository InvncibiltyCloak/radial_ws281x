Introduction
============

This project aims to implement a method for displaying an image on a radially mounted WS281X LED strip. A jpeg image is provided to the program. With no extra effort, this image will be displayed to the rotating display.

Features
--------
- Simple image input system
- Entirely automatic display
- (Future) Synchronization of output to rotation rate of mount

Building
========

This simplest build can be completed by simply running:

    make

All the libraries which are not available as packages on Raspbian have been included in the lib subdirectory. If you would like to update them, simply recompile from source the [`rpi_ws281x`][1] library and the [`jpeg-turbo`][2] library (for its high-level `libjpegturbo` API).

[1]: https://github.com/jgarff/rpi_ws281x
[2]: https://github.com/libjpeg-turbo/libjpeg-turbo
