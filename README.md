pirelay
=======

pirelay is a small add-on board for the [Raspberry Pi](http://www.raspberrypi.org/) for up to 9 temperature controlled relays.  The board's relays are controlled by WS2801 LED drivers which allow for PWM output.  Each relay be a [small on circuit](http://www.allelectronics.com/make-a-store/item/SRLY-19/1A-SOLID-STATE-RELAY-3-8VDC-CONTROL/1.html) relay or a [larger off circuit SSR](http://www.allelectronics.com/make-a-store/item/SRLY-50/50-AMP-SOLID-STATE-RELAY/1.html).  A network of Maxim [One Wire Bus temperature sensors](http://www.maximintegrated.com/datasheet/index.mvp/id/2812) can be connected and assigned to each relay.  The temperature control algorithm can be set to PID, ON/OFF (with compressor delay) or manual mode.

All control and monitoring is done through a web interface.  Temperature and power output data is logged to an SQlite3 database for viewing history charts.

The board is designed for homebrew beer fermentation temperature control but can be easily adapted to other uses.  For beer fermentation a single chest freezer and up to 8 individual fermenters can be controlled from 1 pirelay.

Status
------
As of November 2013 the project is in an early development phase.  The 3rd version of the board ([v2.0d](http://oshpark.com/shared_projects/q5iLnXZY)) has been arrived, but needs to be assembled.  The basic framework of the code is in place to read temp sensors, control relays with PWM, log data to a database and provide a basic API for a more advanced web front end.

Still need to implement true PID mode, move configuration from hardcoding to web interface, build web interface for viewing historical data.

License
-------
The hardware schematic and layout files (Eagle PCB) are included in the repo and licensed under [CERN OHL v.1.1](http://ohwr.org/cernohl)

The software for pirelay is licensed under [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
