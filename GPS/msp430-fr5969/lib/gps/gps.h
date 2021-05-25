/*
 * gps.h
 *
 *  Created on: 27 de jan de 2021
 *  Author: Jose Gustavo
 */

#ifndef __GPS_H__
#define __GPS_H__

#include <lib/gps/nmea/sentence.h>

void send_command(char *command);
void received_command(char *command);
void update_information(char *command, nmeaGPRMC *pack);
void get_position(nmeaGPRMC *pack);

#endif 
