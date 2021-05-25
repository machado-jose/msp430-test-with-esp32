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
import serial

double_code = "double_DS3231.py"
build = "python -m mpy_cross -s -march=xtensa "
DUT_PORT = "/dev/ttyACM1"
DOUBLE_PORT = "/dev/ttyUSB0"
send = "ampy --delay 1 --port "

# Open connect with double one time
print("Connecting to DOUBLE device...")
double_serial = SerialInterface(DOUBLE_PORT, 115200)
double_serial.connect_to_serial()

# From set-up:
# Building, connection and sending phase
try:
    print("Building double code...")
    os.system(build + double_code)
    print("Cleaning the filesystem...")
    double_serial.clean_file_sys()
    print("Sending built double code...")
    os.system(send + DOUBLE_PORT + " put " + double_code)  # .replace(".py",".mpy"))
except:
    sys.exit('fail to upload file(s)')


# Uncomment the next line for not to run the Test
# sys.exit()

double_serial.repl("from double_DS3231 import DOUBLE_DS3231", 0.3)

# Testing Phase
class Test_DS3231(unittest.TestCase):
	# Creates a serial connection and import the classes
	def setUp(self):
		print("\nConnecting to DUT device...")
		self.dut_serial = SerialInterface(DUT_PORT, 115200)
		self.dut_serial.connect_to_serial()

	def test_set_date_time_static(self):
		global double_serial
		print("\nTesting the function that set date and time in the static mode")
		expected_datetime = [2021, 5, 22, 6, 20, 14, 31]  # datatime format: [Year, month, day, weekday, hour, minute, second]
		print("Set value: " + str(expected_datetime))
		# 1 - Objects Creation
		double_serial.repl("ds3231 = DOUBLE_DS3231(21,22)", 0.5)
		# 2 - Input Injection
		self.dut_serial.repl("-t 0 31 14 20 6 22 5 21z", 1) 
		# 3 - Results gathering
		gotten_datetime = double_serial.repl("ds3231.DateTime()", 1.2)
		# 4 - Assertion
		gotten_datetime = gotten_datetime[3].decode()
		print("Gotten value: " + gotten_datetime)
		gotten_datetime | should | equal_to(str(expected_datetime))

	# Using an internal rtc from the double device
	def test_set_date_time_dynamic(self):
		global double_serial
		print("\nTesting the function that set date and time in the dynamic mode")
		expected_datetime = [2021, 5, 22, 6, 20, 14, 31] # datatime format: [Year, month, day, weekday, hour, minute, second]
		my_delay = 2
		print("Set value: "+str(expected_datetime))
		print("Delay time: "+str(my_delay))
		# 1 - Objects Creation
		double_serial.repl("ds3231 = DOUBLE_DS3231(21,22)",0.5)
		# 2 - Input Injection
		self.dut_serial.repl("-t 0 31 14 20 6 22 5 21z", 1)
		double_serial.repl("ds3231.use_internal_rtc()", 0.2)
		sleep(my_delay)
		# This increment was done because of the sleep time - rtc continues counting time
		expected_datetime[6] = expected_datetime[6] + my_delay
		# 3 - Results gathering
		gotten_datetime = double_serial.repl("ds3231.DateTime()",0.5)[3]
		# 4 - Assertion
		gotten_datetime = gotten_datetime.decode()
		print("Gotten value: "+gotten_datetime)
		gotten_datetime |should| equal_to (str(expected_datetime))
	
	def test_get_date_time(self):
		global double_serial
		print("\nTesting the function that get date and time")
		expected_datetime = [2021, 5, 22, 6, 20, 14, 31] # datatime format: [Year, month, day, weekday, hour, minute, second]
		print("Set value: "+str(expected_datetime))
		# 1 - Objects Creation
		double_serial.repl("ds3231 = DOUBLE_DS3231(21,22)",0.5)
		# 2 - Input Injection
		double_serial.repl("ds3231.DateTime("+str(expected_datetime)+")", 0.2)
		# 3 - Results gathering
		gotten_datetime = self.dut_serial.repl("-gz", 1.2)[0]
		# 4 - Assertion
		gotten_datetime = gotten_datetime.decode().split(',')
		gotten_datetime[-1] = '20' + gotten_datetime[-1][:2]
		gotten_datetime.reverse()
		gotten_datetime = [int(x) for x in gotten_datetime]
		gotten_datetime = str(gotten_datetime)
		print("Gotten value: "+gotten_datetime)
		gotten_datetime |should| equal_to (str(expected_datetime))
	
	# Test both set and get time, this case can be used along with a double device or the device itself (autotest)
	def test_set_get_date_time(self):
		global double_serial
		print("\nTesting the functions that set and get date and time")
		expected_datetime = [2021, 5, 22, 6, 20, 14, 31]
		print("Set value: "+str(expected_datetime))
		# 1 - Objects Creation
		double_serial.repl("ds3231 = DOUBLE_DS3231(21,22)",0.5)
		# 2 - Input Injection
		self.dut_serial.repl("-t 0 31 14 20 6 22 5 21z", 1.2)
		sleep(1)
		# 3 - Results gathering
		gotten_datetime = self.dut_serial.repl("-gz", 1.2)[0]
		# 4 - Assertion
		gotten_datetime = gotten_datetime.decode().split(',')
		gotten_datetime[-1] = '20' + gotten_datetime[-1][:2]
		gotten_datetime.reverse()
		gotten_datetime = [int(x) for x in gotten_datetime]
		gotten_datetime = str(gotten_datetime)
		print("Gotten value: "+gotten_datetime)
		gotten_datetime |should| equal_to (str(expected_datetime))

	# closes serial and make the descomissioning
	def tearDown(self):
		global double_serial
		# 5 - descomissioning
		double_serial.repl("ds3231.deinit(); del ds3231; DOUBLE_DS3231", 1)
		self.dut_serial.close_serial()


if __name__ == '__main__':
	unittest.main()
