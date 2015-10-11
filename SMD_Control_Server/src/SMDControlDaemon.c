/*
*  SMDControlDaemon.c
*  AMCI_SMD
*
*	Created by John Davis on 4/16/15. Copyright 2015 3ML LLC
*	Compile with:
*	gcc -I/usr/local/include/modbus -L/usr/local/lib -Wl,-rpath=/usr/local/lib -Wall -lmodbus
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SMD_Constants.h"
#include "SMD_Utilities.h"
#include "SMD_SocketOps.h"

static void daemonize();
static void start_listening();
static void show_usage();
static void parse_args(int argc, char **argv);

int main(int argc, char *argv[]) {
	
	if(geteuid() != 0) {
		fprintf(stderr, "Could not launch SMD server - are you root?\n");
		exit(EXIT_FAILURE);
	}
	
	parse_args(argc, argv);
	
	return 0;
}


/* Functions - Program Command */

void parse_args(int argc, char **argv) {
	
	int opt = 0;
	
	while ((opt = getopt(argc, argv, "dhvp:")) != -1) {
		
		switch (opt) {
				
			case 'd' :
				daemonize();
				start_listening();
				break;
				
			case 'v' :
				VERBOSE = 1;
				start_listening();
				break;
				
			case 'p' :
				if( argc <= 3) {
					fprintf(stderr, "You must specify a run option (-v or -d) with this argument\n");
					exit(EXIT_FAILURE);
				}
				
				else {
					SERVER_PORT = atoi(optarg);
				}
				
				break;
				
			case 'h' :
			case '?' :
				show_usage();
				break;
				
			default:
				show_usage();
				exit(EXIT_FAILURE);
		}
	}
	
	if(argc == 1) {
		show_usage();
	}
}

static void daemonize() {
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

static void start_listening() {
	
	while(1) {
		//open the server socket
		open_server_socket();
	}
}

static void show_usage() {
	fprintf(stderr, "\n");
	fprintf(stderr, "3ML LLC Motion Server for AMCI SMD Products\n");
	fprintf(stderr, "Copyright 2015 3ML LLC\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "-h: display this message\n");
	fprintf(stderr, "-p: specify server port (default 7000)\n");
	fprintf(stderr, "-d: run as daemon\n");
	fprintf(stderr, "-v: run in foreground in verbose mode\n");
	exit(EXIT_FAILURE);
}
