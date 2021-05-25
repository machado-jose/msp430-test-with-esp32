# Do the serial interface between PC and DUT or DOUBLE
from serial_interface_upython import SerialInterface
# Library for testing with different asserts
from should_dsl import should
# Test class from wich our class inhereints
import unittest
# Operational System Interface
import os
import sys
# Utils
import time
from time import sleep

double_code = "double_GPS.py"
build = "python -m mpy_cross -s -march=xtensa "
DUT_PORT = "/dev/ttyACM1"
DOUBLE_PORT = "/dev/ttyUSB0"
send = "ampy --port "

# From set-up:
# Building, connection and sending phase
try:
	print("Building double code...")
	os.system(build+double_code)
	print("Cleaning the filesystem...")
	double_serial = SerialInterface(DOUBLE_PORT, 115200)
	double_serial.connect_to_serial()
	double_serial.clean_file_sys()
	double_serial.close_serial()
	print("Sending built double code...")
	os.system(send+DOUBLE_PORT+" put "+double_code)#.replace(".py",".mpy"))
except:
	sys.exit('fail in set-up phase')
# Uncomment the next line for not to run the Test
# sys.exit()

# Testing Phase
class Test_Template(unittest.TestCase):
	#Creates a serial connection and import the classes
	def setUp(self):
		print('\n')
		print("Connecting to DUT device...")
		self.dut_serial = SerialInterface(DUT_PORT, 115200)
		self.dut_serial.connect_to_serial()
		print("Connecting to DOUBLE device...")
		self.double_serial = SerialInterface(DOUBLE_PORT, 115200)
		self.double_serial.connect_to_serial()
		self.double_serial.repl("from double_GPS import DOUBLE_GPS", 0.1) 

	def test_send_command_configuration(self):
		print("\nTesting the function send_command configuring GPS to send GGA and RMC info")
		expected_command = 'PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0'
		print("Expected command: " + expected_command)
		# 1 - Objects Creation
		self.double_serial.repl("gps = DOUBLE_GPS(22,16,2)",0.2)
		# 2 - Input Injection
		self.dut_serial.repl("y", 1.2)
		# 3 - Results gathering
		gotten_command = self.double_serial.repl("gps.received_command()", 0.8)[2]
		# 4 - Assertion
		gotten_command = gotten_command.decode() 
		gotten_command = gotten_command.replace('\'','')
		print("Gotten command: "+gotten_command)
		gotten_command |should| equal_to (expected_command)

	def test_send_command_update_rate(self):
		print("\nTesting the function send_command and received_command, updating GPS rate to 1 second")
		command = 'PMTK220,1000'
		expected_answer = '$GPRMC,141623.523,A,2143.963,S,04111.493,W,,,301019,000.0,W*7B'
		print("Expected answer: " + expected_answer)
		# 1 - Objects Creation
		self.double_serial.repl("gps = DOUBLE_GPS(22,16,2)",0.2)
		# 2 - Input Injection
		self.dut_serial.repl("t", 0.8)
		self.double_serial.repl("gps.received_command()",0.8)
		sleep(1)
		# 3 - Results gathering
		gotten_answer = self.dut_serial.repl("b", 0.6)[0]
		# 4 - Assertion
		gotten_answer = gotten_answer.decode() # To transform from array bytes to String
		print("Gotten answer: " + gotten_answer)
		gotten_answer |should| equal_to (expected_answer)

	def test_get_position(self):
		print("\nTesting the function get_position")
		expected_latitude = -21.732717
		expected_longitude = -41.191550

		print("Expected answer of Latitude: " + str(expected_latitude))
		print("Expected answer of Longitude: " + str(expected_longitude))
		# 1 - Objects Creation
		self.double_serial.repl("gps = DOUBLE_GPS(22,16,2)",0.2)
		# 2 - Input Injection
		self.dut_serial.repl("t", 0.8)
		self.double_serial.repl("gps.received_command()",0.8)
		sleep(1)
		self.dut_serial.repl("b", 0.6)
		# 3 - Results gathering
		gotten_answer = self.dut_serial.repl("u", 0.8)
		gotten_answer_latitude = gotten_answer[0]
		gotten_answer_longitude = gotten_answer[1]
		# 4 - Assertion
		gotten_answer_latitude = gotten_answer_latitude.decode() # To transform from array bytes to String
		gotten_answer_longitude = gotten_answer_longitude.decode() 
		gotten_answer_latitude = float(gotten_answer_latitude)
		gotten_answer_longitude = float(gotten_answer_longitude)
		print("Gotten answer of Latitude: " + str(gotten_answer_latitude))
		print("Gotten answer of Longitude: " + str(gotten_answer_longitude))
		gotten_answer_latitude |should| close_to(expected_latitude, delta=0.000005)
		gotten_answer_longitude |should| close_to(expected_longitude, delta=0.000005)

	#Closes serial 
	def tearDown(self):
		# 5 Descomissioning
		self.double_serial.repl("gps.deinit(); del gps; del DOUBLE_GPS",0.2)
		self.dut_serial.close_serial()
		self.double_serial.close_serial()
		pass

if __name__ == '__main__':
    unittest.main()
