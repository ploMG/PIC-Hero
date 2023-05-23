# PIC Hero

##  General Info

This project allows the user to select songs and play them off a PIC16F1829. A selection screen is displayed by a 2x16 LCD through I2C that lets users choose which song they would like to play using an analog stick component. After a song is selected, the user will be able to play the notes of the song using the touch sensors on the PIC board. The notes of the song will be highlighted on an 8x8 dot matrix through SPI, where the lit LEDs on the dot matrix signify when to press certain notes. Simultaneously, songs are loaded into a DFPlayer Mini connected to an Arduino Nano 33 BLE Sense that communicates with the PIC microcontroller through UART, which will play the user-selected audio on a connected speaker component.

## Setup

### PIC

The source file main.c can be written to the PIC through snap and is found in the maingame.X project folder.
This project was done using MPLAB X IDE v5.35 and XC8 v1.45 compiler, which can be downloaded from [Microchip](https://www.microchip.com/en-us/tools-resources/archives/mplab-ecosystem). 

### Arduino

The source file has three songs charted:

* (Easy) Foghat - Slow Ride
* (Medium) Foo Fighters - Everlong
* (Hard) Gojira - The Heaviest Matter of the Universe

The mp3 files of these songs were uploaded to a microSD and inserted into the DFPlayer. More info is available on the official [DFRobot page](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299).

The script to write to the Arduino is found in the arduino-DF folder. DFRobotDFPlayerMini library is required.

### (Optional) Case

CAD files to 3D print a holding case for the external components are available in the case folder.

## Samples

https://github.com/ploMG/PIC-Hero/assets/128610413/27b00fba-717c-41e0-9f21-f9e96394e969

> The Heaviest Matter of the Universe (no case)

###  ​

![IMG_7527](https://github.com/ploMG/PIC-Hero/assets/128610413/bc93a5a5-9cb6-405c-a3c0-64b88af74f8f)

> EE 3463 Demo Day, April 20th, 2023 
