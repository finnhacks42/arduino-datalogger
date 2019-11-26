#include <TM74HC595Display.h>
#include <TimerOne.h>

int SCLK = 7;
int RCLK = 6;
int DIO = 5;

TM74HC595Display disp(SCLK, RCLK, DIO);

void setup() {
  Timer1.initialize(1500); // set a timer of length 1500
  Timer1.attachInterrupt(timerIsr); // attach the service routine here
}

void timerIsr()
{
  disp.timerIsr();
}

void loop() {  
  delay(2000);
  disp.clear();
  disp.dispFloat(26.2,2);
}
