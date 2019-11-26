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

#include "DHT.h"


const int tempSensorPin = 7;

DHT dht(tempSensorPin, DHT22); // the tempurature sensor. Connect to digial pin 3. 

void setup() {
  delay(2000);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Serial ready"));
}

void loop() {
  delay(3000);
 
  float h = dht.readHumidity();
  float temp = dht.readTemperature();

  int temp_int = (int)temp;
  int temp_fra = (int)((temp - (float)temp_int)*10); //fraction to one decimal place

  int h_int = (int)h;
  int h_fra = (int)((h - (float)h_int)*10); //fraction to one decimal place
  Serial.print("temp: ");Serial.print(temp);Serial.print(", humidity: ");Serial.println(h);

    float sensor_volt;
    float sensorValue;

   sensorValue = analogRead(A0);
   sensor_volt = sensorValue/1024*5.0;

   Serial.print("sensor_volt = ");
   Serial.print(sensor_volt);
   Serial.println("V");

 
 
  sensorValue = analogRead(A3);
  sensor_volt = sensorValue/1024*5.0;
 
  Serial.print("sensor_volt_alcohol = ");
  Serial.print(sensor_volt);
  Serial.println("V");
    
  
}
