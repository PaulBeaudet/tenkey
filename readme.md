#Tenkey edition of the Neotype project

**Goal**: build a robust chorded keyer platform to experiment with.

**Configurations**

This project can be replicated in various configurations
 -Arduino Uno, Adafruit PWM Shield, Bluefruit EZ-Key HID
 -Arduino Micro/Leonardo, Adafruit PWM Shield
 -Arduino Yun, Adafruit PWM

**Considerations**

As a long term way of supporting others whom want to experiement with chording, the Arduino Micro is the default controller used in the provided code. Using just the micro with a full breadboard and some tact (breadboard) switches is the cheapest way to try this project out. Though if serious, better quality then tact switches are recomended. After using the Yun for a while the realization is that the Yun adds too much complexity to the project, introducing bugs that make the project hard to demonstrate. Technical bugs like the 90 second boot time and inappropriate baud rate that glitches out communication between the two Yun brains. With that said there is still funtionality exclusive to the yun described below.    

Features specific to the Yun involved access to its Linux "side". Wifi is more of an added bonus that may come in handy as development continues. When stacking shields on the Yun be sure to pick up a spare set of [Uno stacking headers](https://www.adafruit.com/product/85)  to give extra clearance for the Ethernet and USB ports. A second for the shield I found helpful. It will work without, be sure to insulate the ports from open leads at the bottom of the shield.

The the EZ-Key is a great device for personal convenience once debugging is done if wireless is the goal. Wires are the most hastle free way to test however.

A serial only approach to connectivity is being considered for Arduino Uno/mini/trinket users. There is some basic code included in this repository, for a python program to intrepert the serial signals into key events. This is ALPHA quality code aka "not well tested"

###Main components list

####Basic Keyer

**Micro Controller**
-[Arduino Micro](https://www.adafruit.com/products/1086): Controller used in the default sketch and primary build can be alternated easily with the Leonardo

**Buttons: **
-[Cherry MX blue](http://www.mouser.com/ProductDetail/CHERRY/MX1A-E1NW/?qs=sGAEpiMZZMsqIr59i2oRcl0OtbIxCyKkAg8zsoDLHg0%3d) : Used in the current primary build ( not breadboard friendly )
-[Medium sized tactical buttons](https://www.adafruit.com/products/1119): A bad choice for this project, but they had been used for most of the intial testing

####Haptic feedback (Optional)

**Power:** [PowerBoost 500c](https://www.adafruit.com/product/1944)  /  [Lipo](https://www.adafruit.com/product/2011) - note that PWM shield and darlington need the extra power and USB alone is currently insufficient to solely power this project. 

**PWM / Pagers:** [Shield](https://www.adafruit.com/products/1411) / [breakout](https://www.adafruit.com/products/815) - Powers 8 [Pager Motors](https://www.adafruit.com/product/1201) , something could be said about going for ten as special character are being achieved in combination with the space and backspace keys. The pagers will need to be driven by a [Darlinton Array](http://www.mouser.com/ProductDetail/STMicroelectronics/ULN2803A/?qs=sGAEpiMZZMvAvBNgSS9LqpP7ived4CP2)

#### Bluetooth (optional / recommended for uno build)

-[Bluefruit EZ-key](https://www.adafruit.com/products/1535) : Note that wireless builds can be more than twice the cost because of the need for a battery and bluetooth component.
