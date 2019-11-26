/*

This example code is in the public domain.

 */


#include "DHT.h"

const int tempSensorPin = 4;
DHT dht(tempSensorPin, DHT22); // the tempurature sensor. Connect to digial pin 3. 

void setup() {
  delay(2000);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
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
}
