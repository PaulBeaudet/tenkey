#Tenkey edition of the Neotype project

**Goal**: build more robust chord keyer platform to experiment with.

**Configurations**

This project can be replicated in various configurations
 -Arduino Uno, Adafruit PWM Shield, Bluefruit EZ-Key HID
 -Arduino Micro/Leonardo, Adafruit PWM Shield
 -Arduino Yun, Adafruit PWM

**Considerations**

Current Development is being carried out on the Yun. Wired connectivity is achieved via USB HID emulation. This method is preferred in testing over bluetooth. 

Note the the EZ-Key is a great device for personal convenience once debugging is done. However the CSR component which powers the bluetooth breakout is closed to licensed customers. This limits future application in Neotype. The spirit of the project is open and where as, sometimes that notion can find compromise in a "toaster like" component, this is one area where nuts and bolts access is important. 

Features specific to the Yun involved access to its Linux "side". Wifi is more of an added bonus that may come in handy as development continues. When stacking shields on the Yun be sure to pick up a spare set of Uno stacking headers to give extra clearance for the Ethernet and USB ports. It will work without, but barely.

A serial only approach to connectivity is being considered. This will will involve an Linux side application that reports key event to X or even better reports key events to a synergy server. This could possibly change the hardware requirements and possibilities significantly. 

###Main components list

**Power:** [PowerBoost 500c](https://www.adafruit.com/product/1944)  /  [Lipo](https://www.adafruit.com/product/2011) - note that PWM shield and darlington need the extra power and USB alone is currently insufficient to solely power this project. 

**PWM / Pagers:** [Shield](https://www.adafruit.com/products/1411) / [breakout](https://www.adafruit.com/products/815) - Powers 8 [Pager Motors](https://www.adafruit.com/product/1201) , something could be said about going for ten as special character are being achieved in combination with the space and backspace keys. The pagers will need to be driven by a [Darlinton Array](http://www.mouser.com/ProductDetail/STMicroelectronics/ULN2803A/?qs=sGAEpiMZZMvAvBNgSS9LqpP7ived4CP2) 

**Buttons: **
-[Cherry MX blue](http://www.mouser.com/ProductDetail/CHERRY/MX1A-E1NW/?qs=sGAEpiMZZMsqIr59i2oRcl0OtbIxCyKkAg8zsoDLHg0%3d) : Going to be used for the next build ( not breadboard friendly)
-[Medium sized tactical buttons](https://www.adafruit.com/products/1119): A bad choice for this project, but they have been used for most of the testing
