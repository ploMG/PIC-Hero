#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

UART DFSerial(digitalPinToPinName(11), digitalPinToPinName(10), NC, NC);  //11 is tx, 10 is rx
DFRobotDFPlayerMini myDFPlayer;

void setup()
{
  DFSerial.begin(9600);   // df player connection
  Serial1.begin(9600);    // UART PIC connection, 1 is tx and 2 is rx on Arduino Nano 33 BLE

  if (!myDFPlayer.begin(DFSerial)) {  //wait till df is online
    delay(100);
    while(true);
  }

  myDFPlayer.volume(20);  //Set volume from 0 to 30
}

void loop() {
  
  if (Serial1.available() > 0) {
    
    int song = Serial1.read();
    
    if (song >= '0' && song <= '9') {
      
      song -= '0';  // Convert ASCII to actual integer
      
      if (song == 1){
        myDFPlayer.play(2);  //Play Slow Ride
        delay(29000);
        myDFPlayer.pause();  //pause the mp3
        Serial1.end();   /*end serial communication*/
        Serial1.begin(9600);
      }
      else if (song == 2){
        myDFPlayer.play(3);  //Play Everlong
        delay(28000);
        myDFPlayer.pause();  //pause the mp3
        Serial1.end();   /*end serial communication*/
        Serial1.begin(9600);
      }
      else if (song == 3){
        myDFPlayer.play(1);  //Play Heaviest Matter
        delay(13750);
        myDFPlayer.pause();  //pause the mp3
        Serial1.end();   /*end serial communication*/
        Serial1.begin(9600);
      }
    }
  }
}