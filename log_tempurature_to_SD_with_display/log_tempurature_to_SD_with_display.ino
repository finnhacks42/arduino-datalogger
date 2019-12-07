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
int debug = 0;

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
  pinMode(chipSelect,OUTPUT);
  // see if the card is present and can be initialized:
  while (!SD.begin(chipSelect)) {
    if (debug) {
      Serial.println("Card failed, or not present");
    }
    delay(10000);
  }
  if (debug == 1){
    Serial.println("Card initialized."); 
  }
}

void loop() {
  
  display.setBrightness(0x0f);
  DateTime now = rtc.now();

  float h = dht.readHumidity();
  float temp = dht.readTemperature();

  display.clear();
  display.setBrightness(0x0f);
  if (error_count < 1){
    display.showNumberDecEx(temp*100,0x40,false);
  } else {
    display.showNumberDecEx(0,0x40,false);
    restart_card();
  }
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
    Serial.print("error_count:");
    Serial.println(error_count);
  }
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  Serial.println(dataFile);
  flush_cashe();
  // if the file is available, write to it:
    if (dataFile) {
    dataFile.println(buf1);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    error_count +=1;
    if (debug == 1){
      Serial.println("error opening datalog.txt");
    }
  }
   
  delay(10000);
}  



//attempting and failing to write a different file stops the card from reporting that datalog.txt is still available when it isn't
// I don't know why attempting to write doesn't work or how this clears out some cache but anyway ...
 bool flush_cashe(){ 
    char filename[12]; // Room for an 7.3 name with a NULL terminator
    int randomNumber = random(1,1000000 );
    sprintf(filename, "%7d.txt", randomNumber);
    Serial.println(filename);
    File dataFile = SD.open(filename,FILE_WRITE);
    if (dataFile){
      delay(1500);
      dataFile.close();
      delay(1500);
      return true; 
    }
   dataFile.close();
   delay(1500);
   return false;
 }

 void clearSD()
{
  byte sd = 0;
  digitalWrite(chipSelect, LOW);
  while (sd != 255)
  {
    sd = SPI.transfer(255);
  }
  digitalWrite(chipSelect, HIGH);
}

void restart_card()
{
  
  SPI.begin();
  delay(10);
  boolean b;
  // *** SD Card **
  b = SD.begin(chipSelect);
  if (!b)
  {
    delay(100);
    clearSD();
    delay(100);
    b = SD.begin(chipSelect);
  }
  if (b) {
    if (debug) {
      Serial.println("SD Card started.");
    }
    error_count = 0;
 } else {
    if (debug) {
      Serial.println("SD Card failed to start!");
    }
 }
}
