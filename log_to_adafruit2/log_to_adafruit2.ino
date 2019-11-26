/*
  Demonstrates how to send some data via the W600 module from an Arduino to AdafruitIO.

  created 12 Oct, 2019
  by Finn Lattimore

  This example code is in the public domain.
*/
#include "w600.h"

SoftwareSerial softSerial(12, 13);
#define SERIAL Serial
#define debug  SERIAL

AtWifi wifi;

const char *TARGET_IP   = "\"52.72.201.158\""; // This is the IP address for AdafruitIO
uint16_t TARGET_PORT = 80;
uint16_t LOCAL_PORT  = 1234;


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


char value[] = "value=44.6\n\n";
void setup()
{
  debug.begin(115200);
#if defined(SAMD21)
  wifi.begin(Serial, 9600);
#else
  wifi.begin(softSerial, 9600);
#endif

  configure_wifi(5,F("dnf"),F("thequickbrownfox"));
  connect_to_AP(5);
  int socket = create_socket(5);

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
  debug.println(F("failed to connect to remote server"));
  return socket;
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
