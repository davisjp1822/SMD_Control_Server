# SMD_Control_Server
## An elegant and speedy cross platform motion controller for [AMCI](www.amci.com) SMD series stepper motors.

Designed as a portable motion controller for AMCI SMD series integrated stepper motors and networked drives, SMD_Control_Server (SMDCS) is a socket-based motion control server that follows a handful of rules:

1. Be fast - SMDCS is written in C, so it is fast and small and can be used on a number of hosts.
2. Be portable - Written using portable ANSI (ish) C, SMDCS has minimal external dependencies, and should compile on all hosts.
3. Be easy to integrate - SMDCS uses a socket interface and takes simple text commands. Control SMD motors with anything from telnet to a snazzy iOS mobile client.

More information on AMCI SMD series steppers may be found on [AMCI's website](http://www.amci.com/stepper-motor-control/integrated-stepper-motor-control-smd23e.asp)

Limitations
------
As of right now, SMDCS works only using Modbus/TCP. If you are willing to help out by reversing Ethernet/IP and contributing to a stack, let me know.

Also, SMDCS only supports the non-DLR (Device Level Ring) AMCI SMD motors. The Modbus implementation on the DLR motors is non-compatible with *libmodbus*. AMCI is working on it...

SMDCS only supports one client connection at a time. One motion controller == one axis of motion. If you want to run multiple axes of motion from one machine, spawn however many SMDCS processes you would like all on different ports.

Feedback
------
This is a labor of love (and insanity). If you have questions, comments, concerns, bugs, rants, etc. please post a bug or start a discussion. Alternatively, you can contact me privately and I will definitely respond.

Disclaimer
------
This SOFTWARE PRODUCT is provided by 3ML LLC "as is" and "with all faults." 3ML LLC makes no representations or warranties of any kind concerning the safety, suitability, inaccuracies, typographical errors, or other harmful components of this SOFTWARE PRODUCT. There are inherent dangers in the use of any software, and you are solely responsible for determining whether this SOFTWARE PRODUCT is compatible with your equipment and other software installed on your equipment. You are also solely responsible for the protection of your equipment and your personnel, and 3ML LLC will not be liable for any damages you may suffer in connection with using, modifying, or distributing this SOFTWARE PRODUCT.

In other words, use this at your own risk. And TEST TEST TEST.

Dependencies
------
[libmodbus](https://github.com/stephane/libmodbus)
[cJSON](https://github.com/kbranigan/cJSON)

A C compiler.

Setup and Installation
------

### Software
1. Install *cJSON* files into *src/cJSON*
2. Install *libmodbus*.
3. Check paths in *build.sh* (the paths for libmodbus will vary depending on what system you are using). Fix build.sh to suit your system.
4. Execute *build.sh*.
5. Your executable is at *src/SMDServer*.

### AMCI Motor
1. READ THE [MANUAL](http://www.amci.com/pdfs/integrated-stepper-control/smd-series-ethernet-integrated-stepper-motor-drives.pdf), paying special attention to the pinout diagrams. The SMD power connector is **NOT** protected against reversed wiring.
2. Download the [AMCI NetConfigurator software](http://www.amci.com/configuration-software/amci-net-configurator.zip) and configure your motor's IP address. Do not forget to set the motor to Modbus/TCP!!
3. Configure your motor in the configuration screen to fit your application.
4. Power cycle the motor.

Creating Motion
------
