# Tests for MSP430FR5969 with ESP32 

This project aims to demonstrate a viable alternative for software testing for microcontrollers from the MSP430FRxxxx family using ESP32 as an external device simulator.

The test techniques used in the project were TDD and HILS and the architecture consists of using three components: the computer, the device under test (DUT) and 
the device that simulates the external device (Double). The DUT device used was the MSP430FR5969 and the DUT device was the ESP32. 
More information about the architecture and codes used in ESP32, visit the [micropython_test_lib](https://github.com/saramonteiro/micropython_test_lib) project developed by Sarah Monteiro. 

Three applications were developed for the MSP430FR5969: a GPS device, a device that configures an external RTC and a device that reads and writes a buffer using the SPI protocol.

Code development for the MSP430FR5969 was performed using Code Composer Studio. When running the test script, the software must already be built into the MSP430FR5969. 
