/*
 
 */
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "DHT.h"
#include <TM1637Display.h>

int CLK = 5;
int DIO = 6;

const int chipSelect = 10;
const int tempSensorPin = 4;
int error_count = 0;
int debug = 1;

TM1637Display display(CLK, DIO);

RTC_DS1307 rtc; // the clock
DHT dht(tempSensorPin, DHT22); // the tempurature sensor. Connect to digial pin 4. 

void setup() {
  delay(2000);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  rtc.begin();
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  if (debug == 1){
    Serial.println("Card initialized."); 
  }
}

void loop() {
  delay(10000);
  display.setBrightness(0x0f);
  DateTime now = rtc.now();

  float h = dht.readHumidity();
  float temp = dht.readTemperature();

  display.clear();
  display.setBrightness(0x0f);
  display.showNumberDecEx(temp*100,0x40,false);
  uint8_t data[] = {0xff};
  data[0] = display.encodeDigit(0x0c);
  display.setSegments(data,1,3);

  int temp_int = (int)temp;
  int temp_fra = (int)((temp - (float)temp_int)*10); //first digit after decimal place

  int h_int = (int)h;
  int h_fra = (int)((h - (float)h_int)*10); //fraction to one decimal place

  char buf1[30];
  const char* fmt = PSTR("%04d-%02d-%02d,%02d:%02d:%02d,%02d.%01d,%02d.%01d");
  sprintf_P(buf1,fmt, now.year(), now.month(),now.day(),
                      now.hour(), now.minute(),now.second(),
                      temp_int, temp_fra, h_int, h_fra);
  if (debug == 1){
    Serial.println(buf1);
  }
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(buf1);
    dataFile.close();
    error_count = 0;
  }
  // if the file isn't open, pop up an error:
  else {
    error_count +=1;
    if (debug == 1){
      Serial.println("error opening datalog.txt");
    }
  }

}
