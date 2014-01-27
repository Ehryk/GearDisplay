GearDisplay
===========

This is an Arduino based project that displays the currently selected gear in a manual transmission vehicle. It uses six Hall-Effect sensors connected to the analog inputs of the arduino, calculates the average, and then uses an adjustable tolerance value to see if any are outside of this tolerance value from the average. Ideally, if none are out of tolerance the transmission is in neutral, if only one is then it is the selected gear, and if more than one is then the tolerance is too low and it displays an Error. 

The key to this setup is one or more magnets on the shifter's metal shaft, and the six hall effect sensors at the approximate locations of the edge of the shaft when engaged in each gear (1-5 and Reverse). Six or more speeds are currently not supported, though less than five are (though the unused hall effect sensors should remain in the circuit providing roughly 2.5V).

**Videos**

 - Basic Operation Video: [Basic Operation Video](https://www.youtube.com/watch?v=FTQq74HFvWU)
 - Menu System in Detail: [Menu System Video](https://www.youtube.com/watch?v=9K3rxGJS8n4)
 - Sensor Voltages: [Sensor Voltages Video](https://www.youtube.com/watch?v=qHEV87LpvVI)

**Core**

This is a display-agnostic version of the core functionality. It will read the analog inputs, determine gear selection, and that's it. It will log detailed information to the Serial port if in debug mode and a device is listening for it. If you have your own display that isn't listed in the versions, start here.

**v1 - 16x2 LCD Display**

This is the first version I wrote, tuned for a standard 16x2 LCD screen. A schematic is included. This uses three push button switches and a potentiometer for the following functions: mode switching, tolerance up, tolerance down, and LCD contrast, respectively. This version has 12 display modes:

1. Basic Display
1. Basic Display - With Accent
1. Advanced Display
1. Advanced Display - With Accent
1. Variables Display (tolerance in voltage)
1. Variables Display (tolerance in internal units [0-1023])
1. Gear Voltages (Debug Only)
1. Gear Values (Debug Only)
1. Fill Mode - fills every position with the character of the gear (N for Neutral, 1 for first, etc.)
1. Enter Menu (press + or -)
1. Enter Log (press + or -)
1. Credits
1. Display off

![v1](https://raw2.github.com/Ehryk/GearDisplay/master/Documentation/Pictures/2.07.2013%20-%20Fritzing%20PCBs/4%20-%20With%20LCD.jpg)

**v2 - 16x2 LCD Display - In Casing**

v1 was made to be mounted with the circuit board visible, but I then found some nice black project cases on eBay. This version is smaller to fit in the enclosure, uses an ultracapacitor keep-alive circuit to keep power to the unit briefly after the vehicle has been turned off for the final EEPROM writes, uses a transistor to selectively power the Hall Effect sensors (optional), and MC14490 hardware switch debouncer (optional). I installed it in a 2003 Jeep Wrangler Rubicon.

![v2 Installed](https://raw2.github.com/Ehryk/GearDisplay/master/Documentation/Pictures/9.06.2013%20-%20Install/6%20-%20Mounting%20on%20Dashboard.jpg)
![v2 Sensor Ring](https://raw2.github.com/Ehryk/GearDisplay/master/Documentation/Pictures/9.06.2013%20-%20Install/5%20-%20Hall%20Effect%20Installation.jpg)

**v3 - Color Touchscreen**

Version 3 uses a color touchscreen for both display and user input, mode switching, and tolerance adjustments. Will come soon.

**Library**

This is where I will put a .cpp library of common features, when I get to it.
