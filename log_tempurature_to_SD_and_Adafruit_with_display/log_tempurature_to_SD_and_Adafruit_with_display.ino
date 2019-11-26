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
#include "DHT.h"
#include "w600.h"
#define debug Serial
#include <TM1637Display.h>

int CLK = 5;
int DIO = 6;

const int chipSelect = 10;
const int tempSensorPin = 4;
const char *TARGET_IP   = "\"52.72.201.158\""; // This is the IP address for AdafruitIO
uint16_t TARGET_PORT = 80;
uint16_t LOCAL_PORT  = 1234;


TM1637Display display(CLK, DIO);

RTC_DS1307 rtc; // the clock
DHT dht(tempSensorPin, DHT22); // the tempurature sensor. Connect to digial pin 4. 

// Use pins 2&3 for UNO and 12&13 for MEGA
SoftwareSerial wifiSerial(12,13); // RX, TX on THIS DEVICE (connect to TX, RX on other device). 

// Object used to represent the wifi module
AtWifi wifi;

void setup() {
  delay(2000);
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Serial ready, starting wifi"));
  
  wifi.begin(wifiSerial,9600);
  Serial.println(F("Wifi module started, starting configuration"));

  configure_wifi(5,F("dnf"),F("thequickbrownfox"));
  Serial.println(F("Wifi module configured, starting clock"));
  
  
  rtc.begin();
  Serial.println(F("Clock started, setting time"));

  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.print(F("Time set, initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("Card initialized."); 
}

void loop() {
  delay(10000);
  display.setBrightness(0x0f);
  DateTime now = rtc.now();
  unsigned long timestamp = now.unixtime();

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
   
  const char* fmt = PSTR("%lu,%d.%d,%d.%d");
  int buf_len = snprintf_P(nullptr,0,fmt,timestamp,temp_int,temp_fra,h_int,h_fra);
  char buf[buf_len];
  sprintf_P(buf,fmt,timestamp,temp_int,temp_fra,h_int,h_fra);
  Serial.println(buf);

  post_temp_data(temp_int,temp_fra);

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


char value[14];// = "value=44.6\n\n"; //putting it inside the function does not work for some reason ...
void post_temp_data(int degree, int dec_degree){
  if (connect_to_AP(1)){
    int socket = -1;
    socket = create_socket(5);
    if (socket > 0){
       sprintf(value,"value=%d.%d\n\n",degree,dec_degree);
        wifi.httpPost(
          socket,
          F("POST /api/v2/finn24/feeds/temp/data HTTP/1.1\n"),
          F("Host: io.adafruit.com\n"),
          F("User-Agent: arduino\n"),
          F("Content-Type: application/x-www-form-urlencoded\n"),
          F("Accept: */*\nX-AIO-Key: 1cee138a9acd409c9b9d52cd79ff7b68\n"),
          value
        );
     }
  }
}

bool configure_wifi(int retries, const __FlashStringHelper* ssid, const __FlashStringHelper* password){
  bool ssid_set = false;
  bool psswd_set = false;
  bool sta_mode_set = false;
  int attempt = 0;
  
  while (!sta_mode_set && attempt < retries) {
    debug.print(F("Configuring wifi, attempt:")); debug.println(attempt + 1);
    wifi.sendAT(F("AT+Z")); //wifi_reset
    delay(1500);
    sta_mode_set = wifi.wifiSetMode(STA);
    attempt ++;
  } if (!sta_mode_set){
      debug.println(F("could not set module to STA mode"));
      return false;
  }

  attempt = 0;
  debug.println(F("setting ssid ..."));
  while (!ssid_set && attempt < retries){
      ssid_set = wifi.wifiStaSetTargetApSsid(ssid); 
      delay(150);
      attempt ++;
  } if (!ssid_set){
    debug.println(F("failed to set ssid"));
    return false;
  }
  attempt = 0;
  debug.println(F("setting password"));
  while (!psswd_set && attempt < retries){
    psswd_set = wifi.wifiStaSetTargetApPswd(password);
    delay(150);
    attempt++;
  } if (!psswd_set){
    debug.println(F("failed to set password"));
    return false;
  }
  debug.println("Wifi configured");
  return true;
}


int create_socket(int retries) {
  int socket = -1;
  for (int attempt = 0; attempt < retries; attempt ++) {
    debug.print(F("Creating socket to remote server, attempt:")); debug.println(attempt + 1);
    socket = wifi.wifiCreateSocketSTA(TCP, Client, TARGET_IP, TARGET_PORT, LOCAL_PORT);
    if (socket >= 0) {
      debug.print(F("connected to remote server. Socket=")); debug.println(socket);
      delay(400);
      return socket;
    }
    delay(1000);
  }
  debug.println(F("failed to create socket to remote server"));
  return -1;
}

bool connect_to_AP(int retries) {
  bool joined = false;
  int attempt = 0;
  while (!joined && attempt < retries) {
    joined = wifi.sendAT(F("AT+WJOIN")); //join network
    delay(1500);
    attempt ++;
  } if (!joined) {
    debug.println(F("failed to join network"));
    return false;
  }
  debug.println(F("connected to AP"));
  return true;
}
