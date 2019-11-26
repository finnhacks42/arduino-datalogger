/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */
//#include <SPI.h>
//#include <Wire.h>

#include <SD.h>
#include "DHT.h"

#include "RTClib.h"
#include <TM74HC595Display.h>
#include <TimerOne.h>
#include "w600.h"

#define DHTPIN 3 
#define DHTTYPE DHT22
#define SERIAL Serial
#define debug  SERIAL

const int chipSelect = 10;
const int ledPin = 2;

int SCLK = 7;
int RCLK = 6;
int DIO = 5;

TM74HC595Display disp(SCLK, RCLK, DIO);
unsigned char LED_0F[29];

DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;

// Define USE_HWSERIAL1 if you have a MEGA or similar and want to use SERIAL1 to communicate with the wifi board
// currently this functionality does not appear to function with the w600 library. 
#if defined(SAMD21) || defined(USE_HWSERIAL1)
   #define wifiSerial Serial1
#else
   // Use 2&3 for UNO and 12&13 for MEGA
   SoftwareSerial wifiSerial(12,13); // RX, TX on THIS DEVICE (connect to TX, RX on other device). 
#endif

AtWifi wifi;

const char *TARGET_IP   = "\"52.72.201.158\""; // This is the IP address for AdafruitIO
uint16_t TARGET_PORT = 80;
uint16_t LOCAL_PORT  = 1234;

int connect_to_AP(int retries){
  bool ssid_set = false;
  bool psswd_set = false;
  bool joined = false;
  int attempt = 0;
  debug.println(F("setting ssid ..."));
  while (!ssid_set && attempt < retries){
      ssid_set = wifi.wifiStaSetTargetApSsid(F("dnf")); //TODO put in wifi name here
      delay(150);
  } if (!ssid_set){
    debug.println(F("failed to set ssid"));
    return 0;
  }
  
  attempt = 0;
  debug.println(F("setting password"));
  while (!psswd_set && attempt < retries){
    psswd_set = wifi.wifiStaSetTargetApPswd(F("thequickbrownfox")); //TODO put in wifi password here
    delay(150);
  } if (!psswd_set){
    debug.println(F("failed to set password"));
    return 0;
  }

  attempt = 0;
  while (!joined && attempt < retries){
      joined = wifi.sendAT(F("AT+WJOIN")); //join network
      delay(1500);
  } if (!joined){
      debug.println(F("failed to join network"));
      return 0;
  }

  debug.println(F("connected to AP"));
  return 1;
}


char value[] = "value=44.6\n\n";
int socket = -1;

void setupSDcard(){
  rtc.begin();
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  Serial.print(F("Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    while (1);
  }
  Serial.println(F("card initialized."));
}

void setup()
{
  // Open serial communications and wait for port to open:
  debug.begin(9600);
  setupDisplay();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  setupSDcard();  
  wifi.begin(wifiSerial,9600);
   
    configure_wifi(5);
    connect_to_AP(5);
    socket = create_socket(5);
 
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

void loop()
{
  delay(2000);
  DateTime now = rtc.now();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int temp = int(round(t));
  
  
  Serial.println(temp);
  disp.clear();
  disp.dispFloat(t,2);
  
  String dataString = String(now.unixtime());
  dataString += ","+String(h) +"," + String(t);
  
  Serial.println(dataString);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(F(":written to card"));
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println(F("error opening datalog.txt"));
  }   
}

int create_socket(int retries){
  int socket = -1;
  for (int attempt = 0; attempt < retries; attempt ++){
    debug.print(F("Creating socket to remote server, attempt:"));debug.println(attempt+1);
    socket = wifi.wifiCreateSocketSTA(TCP,Client,TARGET_IP,TARGET_PORT,LOCAL_PORT);
    if (socket >= 0) {
      debug.print(F("connected to remote server. Socket="));debug.println(socket);
      delay(400);
      return socket;
    }
    delay(1000);
  }
  debug.println(F("failed to connect to remote server"));
  return socket;
}

void configure_wifi(int retries){
  for (int attempt = 0; attempt < retries; attempt ++) {
    debug.print(F("Configuring wifi, attempt:")); debug.println(attempt + 1);
    wifi.sendAT(F("AT+Z")); //wifi_reset
    delay(1500);
    if (wifi.wifiSetMode(STA)){
      debug.println(F("wifi configured"));
      delay(100);
      return;
    }
  }
  debug.println(F("wifi configuration failed"));
}

void setupDisplay() {
  Timer1.initialize(1500); // set a timer of length 1500
  Timer1.attachInterrupt(timerIsr); // attach the service routine here
  LED_0F[0] = 0xC0; //0
  LED_0F[1] = 0xF9; //1
  LED_0F[2] = 0xA4; //2
  LED_0F[3] = 0xB0; //3
  LED_0F[4] = 0x99; //4
  LED_0F[5] = 0x92; //5
  LED_0F[6] = 0x82; //6
  LED_0F[7] = 0xF8; //7
  LED_0F[8] = 0x80; //8
  LED_0F[9] = 0x90; //9
}

void timerIsr()
{
  disp.timerIsr();
}
