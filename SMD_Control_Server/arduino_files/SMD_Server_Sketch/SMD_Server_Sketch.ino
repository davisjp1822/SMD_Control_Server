// John Davis
// Copyright 2015 3ML LLC

#include <Bridge.h>
#include <Process.h>
#include <FileIO.h>

int BLINK_LED = 0;

void setup() {

  //Serial.begin(115200);
  //while(!Serial);
  
  //setup the environment
  Bridge.begin();

  while(!FileSystem.exists("/mnt/sda1/smd_server/SMDServer"))
    delay(1000);
  
  //start the SMD Server.
  Process server;
  server.runShellCommand("export LD_LIBRARY_PATH=/mnt/sda1/smd_server/lib && ifconfig eth1 10.20.6.1 && /mnt/sda1/smd_server/SMDServer -d");
  BLINK_LED = 1;
  
  //turn on the LED to let us know that we are good
  pinMode(13,OUTPUT);
  
  while(BLINK_LED == 1) {
    digitalWrite(13,HIGH);
    delay(150);
    digitalWrite(13,LOW);
    delay(150); 
  }
}

void loop() {

  //if the SMDServer program is not running, stop the LEDs
  
}
