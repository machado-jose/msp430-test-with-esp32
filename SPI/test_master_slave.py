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

double_code = "double_slave.py"
build = "python -m mpy_cross -s -march=xtensa "
DUT_PORT = "/dev/ttyACM1"
DOUBLE_PORT = "/dev/ttyUSB0"
send = "ampy --port "

print("Connecting to DOUBLE device...")
double_serial = SerialInterface(DOUBLE_PORT, 115200)
double_serial.connect_to_serial()

# From set-up:
# Building, connection and sending phase
try:
	print("Building double code...")
	os.system(build+double_code)
	print("Cleaning the filesystem...")
	double_serial.clean_file_sys()
	print("Sending built double code...")
	os.system(send+DOUBLE_PORT+" put "+double_code)#.replace(".py",".mpy"))
except:
	sys.exit('fail to upload file(s)')
# Uncomment the next line for not to run the Test
# sys.exit()

# Testing Phase
class Test_Template(unittest.TestCase):
	#Creates a serial connection and import the classes
	def setUp(self):
		global double_serial
		print('\n')
		print("Connecting to DUT device...")
		self.dut_serial = SerialInterface(DUT_PORT, 115200)
		self.dut_serial.connect_to_serial()
		double_serial.repl("from double_slave import Double_slave", 0.1) 

	def test_reading_registers_without_indicating_address(self):
		global double_serial
		print("\nTesting the function that reading registers without indicating address")
		expected_readings = [1,2,3]
		gotten_readings = []
		print("Expected Readings "+str(expected_readings))
		# 1 - Objects Creation
		double_serial.repl("slave = Double_slave(13,12,14,15)",0.5)
		# 2 - Input Injection
		double_serial.repl("slave.enable_transaction("+str(expected_readings)+")",0.5)
		# 3 - Results gathering
		gotten_readings = self.dut_serial.repl("-g 3z", 1.2)[0]
		gotten_readings = gotten_readings.decode()
		gotten_readings = gotten_readings.split(',')
		gotten_readings = [int(i) for i in gotten_readings]
		# 4 - Assertion
		print("Gotten value: " + str(gotten_readings))
		str(gotten_readings) |should| equal_to (str(expected_readings))

	def test_reading_registers_indicating_address(self):
		global double_serial
		print("\nTesting the function that reading registers indicating address")
		expected_readings = [1,2,3]
		expected_written = [14,0,0,0]
		gotten_readings = []
		print("Expected Readings "+str(expected_readings))
		# 1 - Objects Creation
		double_serial.repl("slave = Double_slave(13,12,14,15)",0.5)
		# 2 - Input Injection
		double_serial.repl("slave.enable_transaction([0,1,2,3])",0.5)
		# 3 - Results gathering
		gotten_readings = self.dut_serial.repl("-a 14 3z", 1.2)[0]
		gotten_readings = gotten_readings.decode()
		gotten_readings = gotten_readings.split(',')
		gotten_readings = [int(i) for i in gotten_readings]
		gotten_written = double_serial.repl("slave.get_received_buffer()",0.5)[2]
		gotten_written = gotten_written.decode()
		# 4 - Assertion
		print("Gotten value: " + str(gotten_readings))
		str(gotten_readings) |should| equal_to (str(expected_readings)) 
		gotten_written |should| equal_to (str(expected_written))

	def test_writing_registers_without_indicating_address(self):
		global double_serial
		print("\nTesting the function that writing registers without indicating address")
		expected_written = [1,2,3]
		gotten_values = []
		print("Expected written values "+str(expected_written))
		# 1 - Objects Creation
		double_serial.repl("slave = Double_slave(13,12,14,15)",0.5)
		# 2 - Input Injection 
		double_serial.repl("slave.enable_transaction([0,0,0])",0.5)
		self.dut_serial.repl("-s 1 2 3z", 1.2)
		# 3 - Results gathering
		gotten_values = double_serial.repl("slave.get_received_buffer()", 0.5)[2]
		gotten_values = gotten_values.decode()
		# 4 - Assertion
		print("Gotten value: "+gotten_values)
		gotten_values |should| equal_to (str(expected_written))

	def test_writing_registers_indicating_address(self):
		global double_serial
		print("\nTesting the function that writing registers indicating address")
		expected_written = [1,2,3]
		address = 14
		gotten_values = []
		print("Expected written values "+str(expected_written))
		# 1 - Objects Creation 
		double_serial.repl("slave = Double_slave(13,12,14,15)",0.5)
		# 2 - Input Injection 
		double_serial.repl("slave.enable_transaction([0,0,0,0])",0.5)
		self.dut_serial.repl("-s 14 1 2 3z", 1.2)
		# 3 - Results gathering
		gotten_values = double_serial.repl("slave.get_received_buffer()",0.5)[2]
		gotten_values = gotten_values.decode()
		# 4 - Assertion
		print("Gotten value: " + gotten_values)
		expected_written.insert(0, address)
		gotten_values |should| equal_to (str(expected_written))

	#closes serial 
	def tearDown(self):
		global double_serial
		double_serial.repl("slave.deinit(); del slave; del Double_slave;", 0.2)
		self.dut_serial.close_serial()

if __name__ == '__main__':
    unittest.main()
