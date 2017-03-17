# SMD_Control_Server
## An elegant and speedy cross platform motion controller for [AMCI](www.amci.com) SMD series stepper motors.

Designed as a portable motion controller for AMCI SMD series integrated stepper motors and networked drives, SMD_Control_Server (SMDCS) is a socket-based motion control server that follows a handful of rules:

1. Be fast - SMDCS is written in C, so it is fast and small and can be used on a number of hosts.
2. Be portable - Written using portable ANSI (ish) C, SMDCS has minimal external dependencies, and should compile on all hosts.
3. Be easy to integrate - SMDCS uses a socket interface and takes simple text commands. Control SMD motors with anything from telnet to a snazzy iOS mobile client.

More information on AMCI SMD series steppers may be found on [AMCI's website](http://www.amci.com/stepper-motor-control/integrated-stepper-motor-control-smd23e.asp)

Click on the screenshot below for a video of the server in action!

[![YouTube Screenshot](https://raw.githubusercontent.com/davisjp1822/SMD_Control_Server/master/Screenshots/youtube_screenshot.png)](https://www.youtube.com/watch?v=lf8ZZJNPgjk)

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

Also, this software is not endorsed or supported in any way by Advanced Micro Controls Inc (AMCI). As such, no warranty is either implied or offered by AMCI. SMD is a trademark of AMCI used with permission.

Dependencies
------
[libmodbus](https://github.com/stephane/libmodbus)

[cJSON](https://github.com/kbranigan/cJSON)

A C compiler (GCC is probably your best bet, honestly)

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
2. Download the [AMCI NetConfigurator software](http://www.amci.com/configuration-software/amci-net-configurator.zip) and configure your motor's IP address. Do not forget to set the motor to **Modbus/TCP**!!
3. Configure your motor in the configuration screen to fit your application.
4. Power cycle the motor.

Creating Motion
------
BEFORE DOING ANYTHING - READ THE DOCUMENTATION at `docs/html/index.html`.

First and foremost, you will want to start SMDCS. Navigate to where the compiled binary is stored (typically at *src/SMDControl*), and execute the following the launch the daemon:

```bash
# must be root (or use sudo) to run SMDServer

# if you want to see console output, specify -v
$ sudo src/SMDServer -v

# if you want to run as a daemon, specify -d
$ sudo src/SMDServer -d

# if you want to specify the port, use -p (default is port 7000)
$ sudo src/SMDServer -v -p 4242
```

Assuming your motor is hooked-up and configured as recommended before, you should be able to telnet to your server and start issuing commands. Some examples would be:

```bash
#assuming your motor IP address is 10.20.6.50

$ telnet localhost 7000
connect,10.20.6.50
SMD_CONNECT_SUCCESS
resetErrors
COMMAND_SUCCESS
jogCW,250,250,5000,0 # jog clockwise with 250 accel, 250 decel, 5000 steps/rev velocity, and 0 jerk
COMMAND_SUCCESS
holdMove # stops the move
COMMAND_SUCCESS
disconnect
```

Of course, full documentation may be found in *docs/html*. I **highly** recommend browsing the docs, as they will provide all of the motion commands available!

