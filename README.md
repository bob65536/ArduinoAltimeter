# ArduinoAltimeter
Do you need knowing altitude with a barometer and an Arduino? Good, I have something for you!  

## What you need
### Hardware
- An Arduino, with the software, taken on the [official](https://www.arduino.cc/en/Main/Software) website. I took a Nano (tested on a 2€ Nano card).  
- An I²C barometer. You can use a BMP280 (more precise) but a BMP085 works well. You can take them at 1-2€ on eBay/AliExpress/Amazon.  
- An I²C screen (I took a LCD HD44780 (16 columns, 2 rows), plus a I²C connector to reduce number of I/O taken on the Arduino). The screen costs 1€ and as much for the I²C connector.   
- A RTC module, such as the DS3231, with a battery. This component will store the date/time and make logs more readable (especially after reset). This one is preferred over DS1307 because it is a bit more precise. You can get that for 1€.  
- Micro SD card for Arduino. With this, you can store logs on a micro SD card, for a better analysis on computer. For 50 cents, this component is yours.  
- A breadboard to put all that these things together, and wires. You can solder these components on a PCB but you should use sockets to avoid damaging the components.  

### Software
To use my program, you will need some librairies. I am sure optimizations are possible but will be issued later. Here are the librairies used (to add them, simply use Arduino IDE and go to Tools > Manage Librairies) and add these ones:
- For the I2C LCD: **hd44780** *(by Bill Perry bperrybap@opensource.billsworld.billandterrie.com)*
- For the BMP085: **Adafruit BMP085 Library** *(Written by Limor Fried/Ladyada for Adafruit Industries. BSD license. _Note_: you will need to make minor changes to the code).*  
- BMP280: [This library](https://github.com/orgua/iLib), downloadable on GitHub, on your folder `librairies`. *(Written by orgua. GPL 2.0)*  
- For DS3231: **Sodaq_DS3231** *(by Kees Bakker -- MIT License)*  
- SD: Created by Tom Igoe (Arduino examples). Public Domain. It is already included in the IDE.  

## How to connect all these components together?
If you use another Arduino card, pinout may be different. Here, the pins are for the Nano card.  

With the I²C bus, things are way simpler. Indeed, for the I²C adapter for the screen, the barometer and the RTC module, you will connect the four pins as such:  
```
Module pin -> Arduino IO
SDA 	   -> A4
SCL		   -> A5
GND		   -> one of the GND
VCC		   -> 3.3V (or 5V*)
```
* _Warning!_ If the component only accepts 3.3V, use the 3.3V pin! I'm not responsible if you fry your components!  

Now, for the SD card, we will use pins 6, 11, 12 and 13 as such:
```
*Module pin -> Arduino IO*
MOSI 	   -> D11
MISO	   -> D12
CLK  	   -> D13
CS  	   -> D6
GND		   -> one of the GND
VCC		   -> 3.3V (or 5V*)
```
Note that if you want to change the place of the CS pin, you will need to change the value of the variable `const int chipSelect`, before the setup function.  

Pictures will arrive later, when I will export data from my phone.  

***
TL;DR: Magic code, for magic purposes.  
