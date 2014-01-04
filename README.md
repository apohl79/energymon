energymon
=========

Monitoring the energy consumption of my house and more

power meter
-----------

My power meter is digital and has a red LED that blinks for every consumed 0.001 kilowatt-hour. I used a photo-diode to read those impulses using a gpio pin on a raspberry pi.

gas meter
---------

To track my gas meter I connected a reed-relais to a different gpio pin. On every impulse 0.01 cubic meters has been consumed.

heating interface
-----------------

I have a Brötje heating and built myself an interface to read the outdoor and bioler temperatures. This was based on:

http://www.mikrocontroller.net/topic/218643
