AM_CFLAGS = -Wall -pedantic -pthread -I/usr/local/include/modbus -I/usr/local/include/cjson --std=gnu99
AM_LDFLAGS = -lcjson -lm -lmodbus 

bin_PROGRAMS = smd_server
smd_server_SOURCES = main.c  SMD_Constants.c  SMD_Constants.h  SMD_Modbus.c  SMD_Modbus.h  SMD_Motor_Commands.c  SMD_Motor_Commands.h  SMD_SocketOps.c  SMD_SocketOps.h  SMD_Utilities.c  SMD_Utilities.h
