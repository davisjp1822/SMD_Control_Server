#!/usr/bin/python

import argparse
import sys
import telnetlib
from socket import error as socket_error
import time

AMCI_HOST = "192.168.1.131"

def main(arguments):

	# create an argparser object and get the arguments from the user
	parser = argparse.ArgumentParser(description="Uses the relativeMove command to move back and forth a specified amount of times", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument("-a", "--acc", type=int, default=250, help="acceleration value (steps/s/s)")
	parser.add_argument("-d", "--dec", type=int, default=250, help="deceleration value (steps/s/s)")
	parser.add_argument("-j", "--jerk", type=int, default=0, help="acceleration jerk (step/s/s/s)")
	parser.add_argument("-v", "--velocity", type=int, default=1000, help="velocity (steps/s)")
	parser.add_argument("-p", "--distance", type=int, default=1000, help="move distance in steps")
	parser.add_argument("-i", "--iterations", type=int, default=4, help="number of moves to make")

	args = parser.parse_args()
	
	if args.iterations == 0:
		print "Iterations cannot be zero!"
		exit(0)

	# ask the user if they want to continue with these values
	answer = ""
	iterationsString = ""

	if args.iterations > 0:
		iterationsString = "%d" % args.iterations

	else:
		iterationsString = "Infinite! Stop with ctrl-c.";

	print "Ready to execute move with these parameters:"
	print "Acceleration (steps/s^2):\t%d" % args.acc
	print "Deceleration (steps/s^2):\t%d" % args.dec
	print "Acceleration Jerk (steps/s^3):\t%d" % args.jerk
	print "Move velocity (steps/s):\t%d" % args.velocity
	print "Move Distance (steps):\t\t%d" % args.acc
	print
	print "Number of iterations: " + iterationsString
	print
	print "*** Don't want any of this? ctrl-c and execute with the -h option for help ***"
	print "*** Want infinite moves? Set iterations (-i) to -1 ***"
	print
	answer = raw_input("Ready to move? (y/n): ")

	if answer == "y":
		relativeMove(args.acc, args.dec, args.jerk, args.velocity, args.distance, args.iterations)

	elif answer == "n":
		exit(0)

	else:
		exit(0) 

def relativeMove(acc, dec, jerk, velocity, distance, i):
	
	tn = telnetlib.Telnet()

	# connect to the local server run by /usr/local/bin/smd_server
	try:
		tn.open("localhost", 7000, 10)
	except socket_error:
		print "Could not connect smd_server at localhost - is the server running?"
		exit(0)

	# once connected via telnet, connect to the AMCI ANG1E or SMD and setup the connection
	try:
		connectString = "connect," + AMCI_HOST + "\n"
		tn.write(connectString)
	except socket_error:
		print "Could not write to AMCI host at %s" % AMCI_HOST
		exit(0)

	if (tn.expect(["SMD_CONNECT_SUCCESS"], 5))[0] == -1:
		print "Could not connect to AMCI host!"
		exit(0)

	# then, reset errors
	try:
		s = "resetErrors" + "\n"
		tn.write(s)
	except socket_error:
		print "Could not write to AMCI host at %s" % AMCI_HOST
		exit(0)

	if (tn.expect(["COMMAND_SUCCESS"], 5))[0] == -1:
		print "Could not execute resetErrors!"
		exit(0)

	# then, preset motor position
	try:
		s = "presetMotorPosition,0" + "\n"
		tn.write(s)
	except socket_error:
		print "Could not write to AMCI host at %s" % AMCI_HOST
		exit(0)

	if (tn.expect(["PRESET_POSITION_SUCCESS"], 5))[0] == -1:
		print "Could not execute presetMotorPosition!"
		exit(0)

	# and then execute for i number of iterations
	# this is a relatively dumb program, and can be improved upon
	# in future versions, it would make sense to actually read the input registers to wait for the motion stopped bit before
	# executing again. For simplicity's sake here, we just wait the move time (distance/speed)+0.5s
	# remember, if i=-1, run forever

	loopVar = i

	if i > 0:
		while(i > 0):
			try:
				s = "relativeMove,%d,%d,%d,%d,%d\n" % (distance, acc, dec, jerk, velocity)
				tn.write(s)
			except socket_error:
				print "Could not write to AMCI host at %s" % AMCI_HOST
				exit(0)

			if (tn.expect(["COMMAND_SUCCESS"], 5))[0] == -1:
				print "Could not execute relativeMove!"
				exit(0)

			time.sleep((distance/velocity)+0.75)
			i = i - 1;

	else:
		while(True):
			try:
				s = "relativeMove,%d,%d,%d,%d,%d\n" % (distance, acc, dec, jerk, velocity)
				tn.write(s)
			except socket_error:
				print "Could not write to AMCI host at %s" % AMCI_HOST
				exit(0)

			if (tn.expect(["COMMAND_SUCCESS"], 5))[0] == -1:
				print "Could not execute relativeMove!"
				exit(0)

			time.sleep((distance/velocity)+0.75)

	print "Done!"

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

