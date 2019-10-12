/*
 *  

 */
#include "seeed_w600.h"

SoftwareSerial softSerial(2,3);
#define SERIAL Serial
#define debug  SERIAL
#define DEBUG_EN 1


#define IO_USERNAME  "finn24"
#define IO_KEY       "1cee138a9acd409c9b9d52cd79ff7b68"


AtWifi wifi;

//const char *TARGET_IP   = "\"192.168.1.12\"";
const char *TARGET_IP   = "\"52.72.201.158\"";
uint16_t TARGET_PORT = 80;
uint16_t LOCAL_PORT  = 1234;
String SSID = "dnf";
String PSWD = "thequickbrownfox";
int32_t socket = -1;

String msg;

void print_recv_buf(uint8_t *buf,uint8_t buf_len)
{
    debug.print("Recv data = ");
    for(int i=0;i<buf_len;i++)
    {
        debug.print(buf[i],HEX);
        debug.print(".");
    }
    debug.println(" ");
    debug.println(" ");
}
static String RSP_OK = "+OK\r\n";
static String RSP_OK_equal = "+OK=";
static const String AT_enter            = "\r\n";


String send_msg = "POST /api/v2/finn24/feeds/temp/data HTTP/1.1\nHost: io.adafruit.com\nUser-Agent: test\nAccept: */*\nX-AIO-Key: 1cee138a9acd409c9b9d52cd79ff7b68\nContent-Length: 8\nContent-Type: application/x-www-form-urlencoded\n\nvalue=35\n\n";




bool wifiSocketSend(ATSerial* at_serial, String &msg,int32_t socket,String &send)
{
    String buff;
    bool ret = false;
    at_serial->flush();
    
    buff += String("AT+SKSND=") + socket + ',' + send.length() + AT_enter;
    ret = at_serial->sendCmdAndGetMsg(msg,buff,RSP_OK_equal,RSP_OK_equal.length());
    if(ret){
        msg.remove(0,4);
    }
    
    const char* b = send.c_str();
    int remain = send.length();
    while (remain > 0){
      int bytes_written = at_serial->ATWrite(b);
      remain -= bytes_written;
      b += bytes_written;
    }
    at_serial->flush();
    return ret;
}





void configure_wifi(int retries){
  for (int attempt = 0; attempt < retries; attempt ++) {
    debug.print("Configuring wifi, attempt:"); debug.println(attempt + 1);
    wifi.wifiReset();
    delay(1500);
    if (wifi.wifiSetMode(STA)){
      debug.println("wifi configured");
      delay(100);
      return;
    }
  }
  debug.println("wifi configuration failed");
}

int connect_to_AP(int retries){
  bool ssid_set = false;
  bool psswd_set = false;
  bool joined = false;
  int attempt = 0;
  debug.println("setting ssid ...");
  while (!ssid_set && attempt < retries){
      ssid_set = wifi.wifiStaSetTargetApSsid(SSID);
      delay(150);
  } if (!ssid_set){
    debug.println("failed to set ssid");
    return 0;
  }
  
  attempt = 0;
  debug.println("setting password");
  while (!psswd_set && attempt < retries){
    psswd_set = wifi.wifiStaSetTargetApPswd(PSWD);
    delay(150);
  } if (!psswd_set){
    debug.println("failed to set password");
    return 0;
  }

  attempt = 0;
  while (!joined && attempt < retries){
      joined = wifi.joinNetwork();
      delay(1500);
  } if (!joined){
      debug.println("failed to join network");
      return 0;
  }

  debug.println("connected to AP");
  return 1;
}

bool create_socket(int retries){
  bool success = false;
  for (int attempt = 0; attempt < retries; attempt ++){
    debug.print("Connecting to remote server, attempt:");debug.println(attempt+1);
    success = wifi.wifiCreateSocket(msg,TCP,Client,TARGET_IP,TARGET_PORT,LOCAL_PORT);
    if (success) {
      socket = atoi(msg.c_str());
      debug.print("connected to remote server. Socket=");debug.println(socket);
      delay(200);
      return true;
    }
    delay(1000);
  }
  debug.println("failed to connect to remote server");
}

void setup()
{
    debug.begin(115200);
    #if defined(SAMD21)
        wifi.begin(Serial,9600);
    #else
        wifi.begin(softSerial,9600);
    #endif

    configure_wifi(5);
    connect_to_AP(5);
    create_socket(5);
 
    //socket = msg[0] - 0x30;
    
    /* Test a echo server.*/
    wifiSocketSend(&wifi, msg, socket, send_msg);
    
//    if(!wifi.wifiSocketSend(msg,socket,send_msg))
//    {
//        debug.println("Socket send failed!!!");
//        return ;
//    }

    debug.println("Wifi connect target server OK,send msg . . .");
    debug.println("message sent");
}

void loop()
{
  
}

void loop2()
{
    static uint32_t count = 0;
    delay(1000);
    if(socket >= 0 ){
        if(wifi.wifiSocketRead(msg,socket,send_msg.length()))
        {
            count++;
            if(msg[0] != '0')
            {
                debug.print("Read from remote server========================================= data = ");
                debug.println(msg);
                debug.println(" ");
                debug.println(" ");
                if(wifi.wifiCloseSpecSocket(socket)){
                    debug.println("Close socket!");
                    while(1);
                }  
            }
            if(count >= 5){
                debug.print("read data failed! ");
                wifi.wifiCloseSpecSocket(socket);
                debug.println("Close socket!");
                while(1);
            
            }
        }
    }
    
}
