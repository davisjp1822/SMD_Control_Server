/*
*  SMDControlDaemon.c
*  AMCI_SMD
*
*	Created by John Davis on 4/16/15. Copyright 2015 3ML LLC
*	Compile with:
*	gcc -I/usr/local/include/modbus -L/usr/local/lib -Wl,-rpath=/usr/local/lib -Wall -lmodbus
*
*/

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <modbus.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include "assert.h"

#include "SMD_Constants.h"
#include "SMD_Utilities.h"
#include "SMD_Motor_Commands.h"

//function declarations - server setup
void daemonize();
void open_server_socket();

int main(int argc, char *argv[]) {
	
	//daemonize();
	
	while(1) {
		//open the server socket
		open_server_socket();
	}
	
	return 0;
}


/* Functions - Socket Control and Command */



/* Functions - Program Command */

void daemonize() {
	pid_t pid;
	
	/* Fork off the parent process */
	pid = fork();
	
	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);
	
	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);
	
	/* On success: The child process becomes session leader */
	if (setsid() < 0)
		exit(EXIT_FAILURE);
	
	/* Catch, ignore and handle signals */
	//TODO: Implement a working signal handler */
	//signal(SIGCHLD, SIG_IGN);
	//signal(SIGHUP, SIG_IGN);
	
	/* Fork off for the second time*/
	pid = fork();
	
	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);
	
	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);
	
	/* Set new file permissions */
	umask(0);
	
	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");
	
	/* Close all open file descriptors */
	int8_t x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
	{
		close (x);
	}
	
	/* Open the log file */
	//openlog("SMD Server", (LOG_CONS|LOG_PERROR|LOG_PID), LOG_DAEMON);
}
