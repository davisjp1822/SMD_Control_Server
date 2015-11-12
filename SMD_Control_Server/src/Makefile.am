AM_CFLAGS = --pedantic -Wall -std=c99 -O2 -pthread
AM_LDFLAGS = -lmodbus -ldl -lm

bin_PROGRAMS = SMDServer
SMDServer_SOURCES = SMD_Constants.c SMD_Utilities.c SMD_Modbus.c SMD_Motor_Commands.c SMD_SocketOps.c SMDControlDaemon.c vendor/cJSON_Utils.c vendor/cJSON.c
