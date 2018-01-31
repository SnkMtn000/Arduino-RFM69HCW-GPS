# Arduino-RFM69HCW-GPS

I stand on the shoulders of giants.

My code/sketches are built on previously shared code by three other contributers.  They are noted at the top at each arduino sketch.  I thank each of you.

Supported board I created and shared at:  https://oshpark.com/shared_projects/vCLw90ht   I drive everything, promini33, rfm69hcw and GPS,  from the pro-mini 3.3v requlator with success from a 9vdc battery, but cannot not recommend this.  I'm using a Ublox NEO-6 GPS Module with great success.  Take care of the ceramic antenna.  It is a brick dangling on a thread and when the center conductor breaks fails to work.  I've repaird mine by stripping back the thin coax and making a very tiny light weight and high gain 1.5 GHz ground plane antenna.

These sketches may also work with the adafruit feather M0 with the noted pin changes in software.  I do not have one and could not test.  (In process).  Check the pin definitions on back of feather.

Three modes supported, Morse, Feld Hell, and RTTY 5N1 170Hz.

Three sketches per mode.  1) a keyboard input mode, a raw beacon mode, and one that transmits GPS telemetry.

For Feld Hell and more I'm using modulation type for register 0x02 = 0x34 of which bit 4 is undefined but works!  Need to flix/clarify this.  Probably should be 0x48.  I need to test.

Good luck.  Enjoy.

73 KC1ENN aka SnkMtn000
