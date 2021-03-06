// John Davis
// Copyright 2015 3ML LLC

#include <Process.h>
#include <FileIO.h>
#include <Console.h>
#include <Bridge.h>

#define ANALOG_INPUT A1
#define DIRECTION_SWITCH 3
#define LED_PIN 13

int LED_ON = 0;
int POT_VALUE = 0;

/* 0 is CW, 1 is CCW */
int JOG_DIRECTION = 0;
String MANUAL_VALUES_FILE_PATH = String("/tmp/smd_manual_values");

void setup() {
  
  //setup the environment
  Bridge.begin();
  Console.begin();
  //Serial.begin(115200);
  
  while(!FileSystem.exists("/mnt/sda1/smd_server/SMDServer"))
    delay(1000);
  
  //start the SMD Server.
  Process server;
  server.runShellCommand("export LD_LIBRARY_PATH=/mnt/sda1/smd_server/lib && ifconfig eth1 10.20.6.2 && /mnt/sda1/smd_server/SMDServer -d");
  LED_ON = 1;

  //create some files that hold our values
  Process manualFile;
  manualFile.runShellCommand("touch /tmp/" + MANUAL_VALUES_FILE_PATH);
  
  //turn on the LED to let us know that we are good
  pinMode(13,OUTPUT);

  pinMode(DIRECTION_SWITCH, INPUT_PULLUP);
}

void loop() {

    //if the SMDServer program is not running, stop the LEDs
    checkLED();

    while(LED_ON == 1) {

      //light goes on!
      digitalWrite(LED_PIN, HIGH);

      //get the values from the inputs
      POT_VALUE = analogRead(ANALOG_INPUT);
      JOG_DIRECTION = digitalRead(DIRECTION_SWITCH);
    
      // print out the value you read:
      Process updateManualFile;
      String commandString = "echo " + String(JOG_DIRECTION)  + "," + String(POT_VALUE) + " > " + MANUAL_VALUES_FILE_PATH; 
      updateManualFile.runShellCommand(commandString);
    
      checkLED();
      delay(1); 
  }

  //if we break out of the loop, it means that the SMD server process is no longer running
  // so turn off the LED.
  digitalWrite(LED_PIN,LOW);

}

void checkLED() {

  Process check;
  check.runShellCommand("pgrep SMDServer");

  //not a valid PID
  if(check.parseInt() > 0) {
    LED_ON = 1;
  }

  else {
    LED_ON = 0;
  }
  
}

