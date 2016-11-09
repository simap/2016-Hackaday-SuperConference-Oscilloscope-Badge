2016 Hackaday SuperConference Osciliscope Badge
====================================================

This hack turns the badge into a 1 channel osciliscope.

Features:

* Trigger rising edge detection at 50%
* Continuous scan mode (currently a compile-time change)
* Configurable timing with 8 steps between 20Ksps (50us/div) and 100sps (10ms/div)
* Compatible with stock kernel and bootloader.

To use, apply a signal of interest to b4 on the expansion port (you should connect GND as well). Press the up arrow to cycle through the various timing modes. The table of timing modes can be changed in the code and defaults to (in samples per second):

1. 100
1. 200
1. 500
1. 1000
1. 2000
1. 5000
1. 10000
1. 20000


TODO/Ideas
========================

* Use the FVR as the ADC ref so that voltages can be read with accurate scale (without something else this limits input to 2.048v). Otherwise its rail to rail with whatever voltage the battery is at.
* Add some basic opamp input circuitry so that positive and negative inputs can be read, and increse the input impedance tolerances. 
* Hack the ADC to support very fast ADC sampling, beyond what is allowed in the datasheet, but at reduced resolution. The display can only display 3 bits, so rates around 150Ksps should be possible.
* Support additional sample rates by changing the timer prescaler on the fly
* Support div scaling and offsets
* Trigger threshold level setting, falling edge, etc.
* Support scrolling around in a larger sample buffer
* Add a touchwheel for better UI control of parameters (esp when scrolling)