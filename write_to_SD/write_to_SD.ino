/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * 
 * 
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
 * 
 * 

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>
#include "RTClib.h"

const int chipSelect = 10;

RTC_DS1307 rtc;

void setup() {
  delay(2000);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Serial ready"));
  rtc.begin();
  Serial.println(F("clock started"));

  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.print(F("Clock set, Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
}


void loop() {
  delay(3000);
  DateTime now = rtc.now();
  unsigned long timestamp = now.unixtime();

  float temp = random(1, 3000) / 100.0; // a dummpy tempuraure between 1 and 30.
  int temp_int = (int)temp;
  int temp_fra = (int)((temp - (float)temp_int)*10); //fraction to one decimal place
   
  const char* fmt = PSTR("%lu,%d.%d");
  int buf_len = snprintf_P(nullptr,0,fmt,timestamp,temp_int,temp_fra);
  char buf[buf_len];
  sprintf_P(buf,fmt,timestamp,temp_int,temp_fra);
  Serial.println(buf);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(buf);
    dataFile.close();
    // print to the serial port too:
    Serial.println(":written to card");
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}
