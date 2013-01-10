GearDisplay
===========

This is an Arduino based project that displays the currently selected gear in a manual transmission vehicle. It uses six Hall-Effect sensors connected to the analog inputs of the arduino, calculates the average, and then uses an adjustable tolerance value to see if any are outside of this tolerance value from the average. Ideally, if none are the transmission is in neutral, if only one is then it is the selected gear, and if more than one is then the tolerance is too low and it displays an Error. 

The key to this setup is one or more magnets on the shifter's metal shaft, and the six hall effect sensors at the approximate locations of the gears (1-5 and Reverse). Six or more speeds are currently not supported.

**Core**

This is a display-agnostic version of the core functionality. It will read the analog inputs, determine gear selection, and that's it. It will log detailed information to the Serial port if in debug mode and a device is listening for it. If you have your own display, start here.

**v1 - 16x2 LCD Display**

This is the first version I wrote, tuned for a standard 16x2 LCD screen. A schematic is included. This uses three push button switches and a potentiometer for mode switching, tolerance up, tolerance down, and LCD contrast. This version has 9 display modes:
1. Basic Display
1. Advanced Display
1. Variables Display (tolerance in voltage)
1. Variables Display (tolerance in internal units [0-1023])
1. Values Display (displays all six internal unit values)
1. Voltage Display (displays all six voltages)
1. Fill Mode - fills every position with the character of the gear (N for Neutral, 1 for first, etc.)
1. Credits
1. Display off

**v2 - Color Touchscreen**

This uses a color touchscreen for both display and user input, mode switching, and tolerance adjustments. Will come soon.
