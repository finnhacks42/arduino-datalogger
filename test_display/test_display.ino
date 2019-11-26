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


#include <TM1637Display.h>

int CLK = 5;
int DIO = 6;

TM1637Display display(CLK, DIO);

void setup() {
  delay(2000);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Serial ready"));
}

int temp_i = 0;
int temp_f = 0;
void loop() {
  delay(1000);

  temp_i = (temp_i + 1) % 50;
  temp_f = (temp_f + 2) % 50;
  float temp = temp_i + float(temp_f)/10.0;
  Serial.println(temp);

  display.clear();
  display.setBrightness(0x0f);
  display.showNumberDecEx(temp*100,0x40,false);
  uint8_t data[] = {0xff};
  data[0] = display.encodeDigit(0x0c);
  display.setSegments(data,1,3);
}
